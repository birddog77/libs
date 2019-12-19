/*
 * drn_mml.h
 * 
 * A basic parser/decoder for MML (Music Macro Language) format music files
 * -    currently works on basic samples but not full featured or thoroughly
 *      tested
 * -    for more info on MML see:
 *      https://shauninman.com/assets/downloads/ppmck_guide.html
 *      http://benjaminsoule.fr/tools/vmml/
 * -    uses standard math.h and includes Sean Barrett's stretchy_buffer
 *      library (https://github.com/nothings/stb)
* 
// Version History
// 0.6  (2019-12-07)    Wave definitions, measure cycling
// 0.4  (2018-05-04)    Initial release
//
//
// LICENSE
//
//   See end of file
*/

#ifndef __INCLUDED__DRN_MML_H__
#define __INCLUDED__DRN_MML_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
    double length;
    double accum_time;
    float frequency;
} mml_note_t;

typedef struct {
    float speed_multiplier;
    double accum_time;
    unsigned int * track_pos;
} mml_decode_state_t;

typedef struct {
    unsigned int beats_per_minute;
    double length;
    double volume;
    unsigned int track_count;
    unsigned int * waves;     // indexes into wavetable
    mml_note_t ** tracks;
} mml_data_t;

typedef struct {
    double note_length;                 /* default is .25   */
    double hit_length;                  /* default is .75   */
    unsigned int octave;                /* default is 4     */
} mml_read_state_t;

typedef struct {
    mml_decode_state_t decode_state;
    mml_data_t data;
} drn_mml_t;

/* function prototypes */
drn_mml_t* drn_mml_open_file(const char*);
drn_mml_t* drn_mml_open_mem(const char*,unsigned int);
void drn_mml_free(drn_mml_t*);
void drn_mml_reset_decode_state(drn_mml_t*);
double drn_mml_decode_stream(drn_mml_t* m,double dt);


#ifdef __cplusplus
}
#endif


#endif /* __INCLUDED__DRN_MML_H__ */

/* ////////////////////////////////////////////////////////////////////
 * 
 * 
 * 
                IMPLEMENTATION                  ///////////////////////
                * 
                * 
                * 
//////////////////////////////////////////////////////////////////// */
#ifdef DRN_MML_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

/* include stretchy buffer stuff */
#ifndef STB_STRETCHY_BUFFER_H_INCLUDED
#define STB_STRETCHY_BUFFER_H_INCLUDED

#ifndef NO_STRETCHY_BUFFER_SHORT_NAMES
#define sb_free   stb_sb_free
#define sb_push   stb_sb_push
#define sb_count  stb_sb_count
#define sb_add    stb_sb_add
#define sb_last   stb_sb_last
#endif

#define stb_sb_free(a)         ((a) ? free(stb__sbraw(a)),0 : 0)
#define stb_sb_push(a,v)       (stb__sbmaybegrow(a,1), (a)[stb__sbn(a)++] = (v))
#define stb_sb_count(a)        ((a) ? stb__sbn(a) : 0)
#define stb_sb_add(a,n)        (stb__sbmaybegrow(a,n), stb__sbn(a)+=(n), &(a)[stb__sbn(a)-(n)])
#define stb_sb_last(a)         ((a)[stb__sbn(a)-1])

#define stb__sbraw(a) ((int *) (a) - 2)
#define stb__sbm(a)   stb__sbraw(a)[0]
#define stb__sbn(a)   stb__sbraw(a)[1]

#define stb__sbneedgrow(a,n)  ((a)==0 || stb__sbn(a)+(n) >= stb__sbm(a))
#define stb__sbmaybegrow(a,n) (stb__sbneedgrow(a,(n)) ? stb__sbgrow(a,n) : 0)
#define stb__sbgrow(a,n)      (*((void **)&(a)) = stb__sbgrowf((a), (n), sizeof(*(a))))

#include <stdlib.h>

static void * stb__sbgrowf(void *arr, int increment, int itemsize)
{
   int dbl_cur = arr ? 2*stb__sbm(arr) : 0;
   int min_needed = stb_sb_count(arr) + increment;
   int m = dbl_cur > min_needed ? dbl_cur : min_needed;
   int *p = (int *) realloc(arr ? stb__sbraw(arr) : 0, itemsize * m + sizeof(int)*2);
   if (p) {
      if (!arr)
         p[1] = 0;
      p[0] = m;
      return p+2;
   } else {
      #ifdef STRETCHY_BUFFER_OUT_OF_MEMORY
      STRETCHY_BUFFER_OUT_OF_MEMORY ;
      #endif
      return (void *) (2*sizeof(int)); /* try to force a NULL pointer exception later */
   }
}
#endif /* STB_STRETCHY_BUFFER_H_INCLUDED */

/* use these to index the note frequency table */
typedef enum {
    NOTE_C,
    NOTE_C_SHARP_D_FLAT,
    NOTE_D,
    NOTE_D_SHARP_E_FLAT,
    NOTE_E,
    NOTE_F,
    NOTE_F_SHARP_G_FLAT,
    NOTE_G,
    NOTE_G_SHARP_A_FLAT,
    NOTE_A,
    NOTE_A_SHARP_B_FLAT,
    NOTE_B,
    NOTE_none = -1
} NOTE;

typedef enum {
    SQUARE_ONE_EIGHTH,
    SQUARE_QUARTER,
    SQUARE_HALF,
    SQUARE_THREE_QUARTER,
    TRIANGLE,
    NOISE,
    SINE,
    SAWTOOTH,
    //~ CHORUS,
    //~ BIGDIP,
    NUM_VOICES
} VOICE;

enum {
    MINUS = -1, 
    NONE = 0, 
    PLUS = 1
};

static unsigned char mml_wavetable[NUM_VOICES][16] = {
    {15, 15,   0, 0, 0, 0, 0, 0, 0, 0,    0,    0,    0,    0,    0,    0},                    // square-one-eighth
    {15,15,15,15,0,0,0,0,0,0,0,0,0,0,0,0},                  // square-quarter
    {15,15,15,15,15,15,15,15,0,0,0,0,0,0,0,0},              // square-half
    {15,15,15,15,15,15,15,15,15,15,15,15,0,0,0,0},          // square-three-quarter
    {2,4,6,8,9,11,13,15,14,12,10,8,5,3,1,0},                // triangle
    {8,10,12,9,7,1,3,0,6,15,2,4,11,14,13,5},                // noise
    {0,1,3,5,9,12,13,15,15,13,12,9,5,3,1,0},                // sine
    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},                // sawtooth
    //~ {0,5,11,15,11,5,1,7,8,1,5,11,15,12,7,1},                // chorus
    //~ {0,5,11,15,15,15,15,15,15,15,15,15,15,12,7,1}           // bigdip
};

static float mml_note_frequencies[108] = {
	16.35 	,
 	17.32 	,
	18.35 	,
  	19.45 	,
	20.60 	,
	21.83 	,
   23.12 	,
	24.50 	,
   25.96 	,
	27.50 	,
  	29.14 	,
	30.87 	,
	32.70 	,
  	34.65 	,
	36.71 	,
  	38.89 	,
	41.20 	,
	43.65 	,
  	46.25 	,
	49.00 	,
  	51.91 	,
	55.00 	,
  	58.27 	,
	61.74 	,
	65.41 	,
  	69.30 	,
	73.42 	,
  	77.78 	,
	82.41 	,
	87.31 	,
  	92.50 	,
	98.00 	,
  	103.83 	,
	110.00 	,
  	116.54 	,
	123.47 	,
	130.81 	,
  	138.59 	,
	146.83 	,
  	155.56 	,
	164.81 	,
	174.61 	,
  	185.00 	,
	196.00 	,
  	207.65 	,
	220.00 	,
  	233.08 	,
	246.94 	,
	261.63 	,
  	277.18 	,
	293.66 	,
  	311.13 	,
	329.63 	,
	349.23 	,
  	369.99 	,
	392.00 	,
  	415.30 	,
	440.00 	,
  	466.16 	,
	493.88 	,
	523.25 	,
  	554.37 	,
	587.33 	,
  	622.25 	,
	659.25 	,
	698.46 	,
  	739.99 	,
	783.99 	,
  	830.61 	,
	880.00 	,
  	932.33 	,
	987.77 	,
	1046.50 	,
  	1108.73 	,
	1174.66 	,
  	1244.51 	,
	1318.51 	,
	1396.91 	,
  	1479.98 	,
	1567.98 	,
  	1661.22 	,
	1760.00 	,
 	1864.66 	,
	1975.53 	,
	2093.00 	,
  	2217.46 	,
	2349.32 	,
  	2489.02 	,
	2637.02 	,
	2793.83 	,
  	2959.96 	,
	3135.96 	,
  	3322.44 	,
	3520.00 	,
  	3729.31 	,
	3951.07 	,
	4186.01 	,
  	4434.92 	,
	4698.63 	,
  	4978.03 	,
	5274.04 	,
	5587.65 	,
  	5919.91 	,
   6271.93 	,
  	6644.88 	,
   7040.00 	,
  	7458.62 	,
   7902.13 	
};

static double mml_quant_values[9] = 
    { 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.75, 1.0 };

#define NULLCHAR                '\0'
#define BITS_PER_NOTE           15.0
#define DRN_GET_DECIMAL(f)      (f-floor(f))
#define DRN_PI                  3.14159265359
#define DRN_PI_twice            DRN_PI*2.0
#define DRN_PI_inv              0.31831
#define DRN_PI_inv_twice        0.15915
#define DRN_SQUARE(x)           (sin(x) > 0 ? 1.0 : -1.0)
#define DRN_SAMPLE_WAVETABLE(t,voice,note) \
        mml_wavetable[voice][(int)(roundf(BITS_PER_NOTE*DRN_GET_DECIMAL(note*t)))]
#define DRN_NOTE_LOOKUP(t,voice,note) \
        (((double)(DRN_SAMPLE_WAVETABLE(t,voice,note)) \
        / BITS_PER_NOTE ) * 2.0 - 1.0 ) * 0.90
#define DRN_ONE_NOTE(t,note)    ( 0.99999*DRN_SQUARE(note*DRN_PI_twice*t) )

static const char* mml_buf = NULL;
static unsigned int mml_index = 0;
//~ static double mml_length_counter = 0.0;
static double mml_sequence_counter = 0.0;

void mml__skipwhite_and_nums_s()
{
    int c;
    while(  (c = mml_buf[mml_index++]) == ' ' 
            || c == '\n'
            || c == '\t'
            || (c < 58 && c > 47)   )
        ;                       /* discard whitespace and spurious numbers */
    mml_index--;
}

void mml__skipwhite_s()
{
    int c;
    while(  (c = mml_buf[mml_index++]) == ' ' 
            || c == '\n'
            || c == '\t'   )
        ;                       /* discard whitespace */
    mml_index--;
}

void mml__skipline_s()
{
    int c;
    while(  (c = mml_buf[mml_index++]) != '\n'
            && c != NULLCHAR     )
        ;                       /* discard current line */
    if( c == NULLCHAR ) mml_index--;
}


int mml__get_token_s()
{
    int c;
    mml__skipwhite_and_nums_s();
    while( 1 ) 
    {
        c = mml_buf[mml_index++];
        
        switch( c ) {
            case 'a':   /* notes --         */
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
            case 'g':
            case 'r':
            case 'p':
            case 'o':   /* state mods --    */
            case 'l':
            case 'q':
            case '<':
            case '>':
            case 'w':
            case ';':   /* track finish     */
            case '/':   /* comment          */
            case NULLCHAR:
                return c;
                break;
            default:
                break;
        }
        mml__skipwhite_and_nums_s();
    }
}

int mml__clamp(int i, int min, int max)
{
    int r = i;
    if( i < min ) r = min;
    if( i > max ) r = max;
    return r;
}

double mml__modf(double n)
{
    return (n - (int)n);
}

int mml__get_note_modifier_s() {
    int result = NONE;
    int c;
    mml__skipwhite_s();
    
    c = mml_buf[mml_index++];
    
    if( c == '+' )
        result = PLUS;
    else if( c == '-' )
        result = MINUS;
    else
        mml_index--;
    
    return result;
}

int mml__ipow(int b,int p)
{
    int n = 1;
    int r = b;
    if( p == 0 )
        return 1;
    else
    {
        assert( p>0 );
        while( n<p )
        {
            r *= b;
            ++n;
        }
    }
    return r;
}

int mml__get_num_modifier_s()
{
    mml__skipwhite_s();
    int c,i;
    int sum = 0;
    int count = 0;
    int nums[4];
    
    while(      (c = mml_buf[mml_index++]) < 58      /* tests ascii val of c */
            &&  c > 47
            &&  count < 4                       )
    {
        nums[count++] = c-48;           /* convert from ascii val to a number */
        sum = 0;
        for( i=0; i<count; i++ )
        {
            sum += (nums[count-i-1] * (int)mml__ipow(10,i));
        }
    }
    
    mml_index--;
    
    if( count > 0 )
        return sum;
    else
        return -1;
}

double mml__get_note_length_s()
{
   int n,d;
 
   n = mml__get_num_modifier_s();
   
   if(   mml_buf[mml_index] == '/' &&
         mml_buf[mml_index+1] != '/' )
   {  // slash present, treat n as a numerator
      mml_index += 1;
      d = mml__get_num_modifier_s();
      if( n < 0 )
      {  // no n, default to 1
         return 1.0/(double)d;
      }
      else
      {  // n and d present, return a fraction
         printf("total %d %d %f\n",n,d,((double)n/(double)d));
         return ((double)n/(double)d);
      }
   }
   else
   {  // no slash, n is a fraction
      if( n < 0 )
      {  // no n, use default note length
         return -1.0;
      }
      else
      {  // n present
         return 1.0/(double)n;
      }
   }
}

NOTE mml__fetch_note(int note,int mod)
{
    switch( note ) {
        case 'a':
            return (NOTE)mml__clamp(9 + mod,0,11);
            break;
        case 'b':
            return (NOTE)mml__clamp(11 + mod,0,11);
            break;
        case 'c':
            return (NOTE)mml__clamp(0 + mod,0,11);
            break;
        case 'd':
            return (NOTE)mml__clamp(2 + mod,0,11);
            break;
        case 'e':
            return (NOTE)mml__clamp(4 + mod,0,11);
            break;
        case 'f':
            return (NOTE)mml__clamp(5 + mod,0,11);
            break;
        case 'g':
            return (NOTE)mml__clamp(7 + mod,0,11);
            break;
        case 'r':
        case 'p':
            return (NOTE)NOTE_none;
            break;
        default: break;
    };
    return NOTE_none;
}


const char* mml__read_file(const char* fn,unsigned int* sz)
{
    FILE* f = fopen(fn, "rb");
    assert( f != NULL );
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);

    char* string = (char*)malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    
    
    *sz = fsize;
    
    return (const char*)string;
}

void drn_mml_reset_decode_state(drn_mml_t* m)
{
   int i,j;
   for( i=0; i<m->data.track_count; i++ )
   {
      /* zero track positions */
      m->decode_state.track_pos[i] = 0;
      /* set initial notes to 0 */
      int n = sb_count(m->data.tracks[i]);
      for( j=0; j<n; j++ )
      {
         m->data.tracks[i][j].accum_time = 0.0;
      }
   }
    /* zero total track time */
    m->decode_state.accum_time = 0.0;
    
}

double drn_mml_decode_stream(drn_mml_t* m,double dt)
{
    int i,j;
    double r,v,c;
    mml_note_t * n;
    r = 0.0;
    v = m->data.volume;
    
    m->decode_state.accum_time += dt;
    /* check if we reached the end of the song */
    if( m->decode_state.accum_time > m->data.length )
    {
        dt = m->decode_state.accum_time - m->data.length;
        drn_mml_reset_decode_state(m);
        m->decode_state.accum_time += dt;
    }
    
        
    for( i=0; i<m->data.track_count; ++i )
    {
        j = m->decode_state.track_pos[i];
        n = &m->data.tracks[i][j];
        if( j == sb_count(m->data.tracks[i]) )
            continue;
        else if( n->accum_time+dt > n->length )
        {
            dt = (n->accum_time+dt) - n->length;
            j++;
            m->decode_state.track_pos[i] = j;
            if( j == sb_count(m->data.tracks[i]) )
                continue;
            n = &m->data.tracks[i][j];
            n->accum_time = dt;
        }
        else
            n->accum_time += dt;
        
        if( n->frequency )
        {
            c = DRN_NOTE_LOOKUP(m->decode_state.accum_time,m->data.waves[i],n->frequency);
            
            r += (v*c);
        }
    }
    
    return r;
}




void drn_mml_free(drn_mml_t* m)
{
    int i;
    for( i=0; i<m->data.track_count; i++ )
        sb_free(m->data.tracks[i]);
    sb_free(m->data.tracks);
    sb_free(m->decode_state.track_pos);
    
    free(m);
}



drn_mml_t* drn_mml_open_file(const char* filename)
{
    if( mml_buf )
        free((void*)mml_buf);
        
    unsigned int sz;
    mml_buf = mml__read_file(filename,&sz);
    
    drn_mml_t* song = drn_mml_open_mem(mml_buf,sz);
    
    return song;
}

drn_mml_t* drn_mml_open_mem(const char* buf,unsigned int sz)
{
    int c;
    double rest_len;
    
    mml_buf = buf;
    mml_index = 0;
    
    //~ mml_length_counter = 0;
    mml_sequence_counter = 0;
    
    drn_mml_t* song = (drn_mml_t*)malloc(sizeof(drn_mml_t));
    song->data.beats_per_minute = 140;
    song->data.length = 0.0;
    song->data.tracks = NULL;
    //~ sb_push(song->data.tracks,(mml_note_t*)malloc(sizeof(mml_note_t*)));
    
    song->data.track_count = 0;
    song->decode_state.track_pos = NULL;
    song->decode_state.accum_time = 0.0;
    song->data.volume = 0.0;
    song->data.waves = NULL;
    
    int wave_define = 1;
    mml_read_state_t * rs = NULL;
    double * ms_length = NULL;
    
    int current_track = 0;
    int n,i,m;
    double nl;
    
    /* main parser loop */
   while( mml_buf[mml_index] != NULLCHAR && mml_index < sz )
   {
      c = mml__get_token_s();
      switch( c ) {
         case 'w': /* define wave */
            if( wave_define )
            {
               song->data.track_count += 1;
               current_track = song->data.track_count - 1;
               
               //~ song->data.tracks[current_track] = NULL;
               song->data.volume += 1.0;
               
               n = mml__get_num_modifier_s();
               sb_push(song->data.waves,mml__clamp(n,0,NUM_VOICES-1));
               
               mml_read_state_t tmp;
               tmp.note_length = .25;
               tmp.hit_length = .75;
               tmp.octave = 4;
               
               sb_push(rs,tmp);
               sb_push(ms_length,0.0);
            }
            break;
         case '/':   /* comment */
            if( (c = mml_buf[mml_index++]) == '/' )      /* check for follow '/' */
                 mml__skipline_s();
            break;
         case ';':   /* end current track and start new track */
         {
            if( wave_define )
            {  /* we finish defining waves and reset counters */
               wave_define = 0;
               current_track = 0;
               mml_sequence_counter = 0.0;
               
               /* allocate pointers for parallel tracks */
               sb_add(song->data.tracks,song->data.track_count);
               for( i=0; i<song->data.track_count; i++ )
                  song->data.tracks[i] = NULL;
            }
            else
            {
               ms_length[current_track] += mml_sequence_counter;
               mml_sequence_counter = 0.0;
               
               current_track += 1;
               current_track %= song->data.track_count;
            }
         }
            break;
         case 'l':   /* note length */
         {
            if( (n = mml__get_num_modifier_s()) > 0 )
            {  
               rs[current_track].note_length = 1.0/(double)n;
            }
         }
            break;
         case 'o':   /* note octave */
             if( (n = mml__get_num_modifier_s()) != -1 )
                 rs[current_track].octave = mml__clamp(n,0,8);
             break;
         case '<':   /* octave shift up */
             rs[current_track].octave = mml__clamp(rs[current_track].octave+1,0,8);
             break;
         case '>':   /* octave shift dn */
             rs[current_track].octave = mml__clamp(rs[current_track].octave-1,0,8);
             break;
         case 'q':   /* note hit length */
             if( (n = mml__get_num_modifier_s()) != -1 )
                 rs[current_track].hit_length = mml_quant_values[mml__clamp(n,0,8)];
             break;
         case 'a':   /* notes */
         case 'b':
         case 'c':
         case 'd':
         case 'e':
         case 'f':
         case 'g':
         case 'r':
         case 'p':
         {
            m = mml__get_note_modifier_s();
            i = mml__fetch_note(c,m);
            nl = mml__get_note_length_s();
             
            mml_note_t note;
            note.frequency = (i == -1) ? 0.0 : mml_note_frequencies[12*rs[current_track].octave+i];
            note.length = ( nl < 0 ) ? rs[current_track].note_length : nl;
            
            mml_sequence_counter += note.length;
                         
            rest_len = (1.0-rs[current_track].hit_length)*rs[current_track].note_length;
            note.length -= rest_len;
            note.accum_time = 0.0;
            sb_push(song->data.tracks[current_track],note);
             
            if(  rs[current_track].hit_length < 1.0 )
            {
               mml_note_t rest;
               rest.frequency = 0.0;
               rest.length = rest_len;
               rest.accum_time = 0.0;
               sb_push(song->data.tracks[current_track],rest);
            }
         }
            break;
         default:
            break;
        };
    }
    
   free((void*)mml_buf);
   mml_buf = NULL;
   mml_index = 0;

   song->data.volume = 1.0/song->data.volume;
   song->data.length = 0.0;
   for( i=0; i<song->data.track_count; i++ )
   {
      if( ms_length[i] > song->data.length )
      {
         song->data.length = ms_length[i];
      }
   }
   
   
   for( i=0; i<song->data.track_count; i++ )
   {
      
      
      if( ms_length[i] < song->data.length )
      {
         mml_note_t rest;
         rest.frequency = 0.0;
         rest.length = song->data.length - ms_length[i];
         rest.accum_time = 0.0;
         
         
         sb_push(song->data.tracks[i],rest);
      }
   }
   
      
   sb_add(song->decode_state.track_pos,song->data.track_count);
   for( i=0; i<sb_count(song->decode_state.track_pos); i++ )
      song->decode_state.track_pos[i] = 0;
   
   sb_free(ms_length);
   sb_free(rs); 
    
   return song;
}


#pragma GCC diagnostic pop

#endif /* DRN_MML_IMPLEMENTATION */

/*
------------------------------------------------------------------------------
 Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this 
software, either in source code form or as a compiled binary, for any purpose, 
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this 
software dedicate any and all copyright interest in the software to the public 
domain. We make this dedication for the benefit of the public at large and to 
the detriment of our heirs and successors. We intend this dedication to be an 
overt act of relinquishment in perpetuity of all present and future rights to 
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN 
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION 
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/
