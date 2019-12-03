
/*
//
// LICENSE
//
//   See end of file
*/

#ifndef __INCLUDED__MMS_H__
#define __INCLUDED__MMS_H__


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
   unsigned int   m_share;
   double         hit_length;
   float          frequency;
} mms_note_t;

typedef struct {
   unsigned int   wavetable;
   int            note_count;
   mms_note_t *   notes;
} mms_measure_t;

typedef struct {
   float            speed_multiplier;
   double           accum_time;
   unsigned int *   current_measure;
} mms_decode_state_t;

typedef struct {
   unsigned int         beats_per_minute;
   double               length;
   double               volume;
   unsigned int         wave_count;
   unsigned int *       waves;
   unsigned int         measure_count;
   mms_measure_t **     measures;
} mms_data_t;

typedef struct {
   double            hit_length;                 /* default is .75   */
   unsigned int      octave;                     /* default is 4     */
   int               pre_read;
   unsigned int      wave_index;
} mms_read_state_t;

typedef struct {
   mms_decode_state_t decode_state;
   mms_data_t data;
} mms_t;

/* function prototypes */
mms_t* mms_open_file(const char*);
mms_t* mms_open_mem(const char*);
void mms_free(mms_t*);
void mms_reset_decode_state(mms_t*);
double mms_decode_stream(mms_t* m,double dt);




#ifdef __cplusplus
}
#endif


#endif /* __INCLUDED__MMS_H__ */




// implementation
#ifdef MMS_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"


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
    NUM_VOICES
} VOICE;

enum {
    MINUS = -1, 
    NONE = 0, 
    PLUS = 1
};

static unsigned char mms_wavetable[NUM_VOICES][16] = {
    {15,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0},                    // square-one-eighth
    {15,15,15,15,0,0,0,0,0,0,0,0,0,0,0,0},                  // square-quarter
    {15,15,15,15,15,15,15,15,0,0,0,0,0,0,0,0},              // square-half
    {15,15,15,15,15,15,15,15,15,15,15,15,0,0,0,0},          // square-three-quarter
    {0,2,4,6,9,11,13,15,15,13,11,9,6,4,2,0},                // triangle
    {8,10,12,9,7,1,3,0,6,15,2,4,11,14,13,5}                 // noise
};

static float mms_note_frequencies[108] = {
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

static double mms_quant_values[9] = 
    { 0.15, 0.25, 0.3, 0.45, 0.6, 0.75, 0.8, 0.9, 1.0 };

#define NULLCHAR                '\0'
#define GET_DECIMAL(f)      (f-floor(f))
#define PI                  3.14159265359
#define PI_twice            PI*2.0
#define PI_inv              0.31831
#define PI_inv_twice        0.15915
#define SQUARE(x)           (sin(x) > 0 ? 1.0 : -1.0)
#define SAMPLE_WAVETABLE(t,voice,note) \
        mms_wavetable[voice][int(roundf(15.f*GET_DECIMAL(note*t)))]
#define NOTE_LOOKUP(t,voice,note) \
        ((double(SAMPLE_WAVETABLE(t,voice,note)) \
        / 15.0 ) * 2.0 - 1.0 ) * 0.90
#define ONE_NOTE(t,note)    ( 0.99999*SQUARE(note*PI_twice*t) )


static const char* mms_buf = NULL;
static unsigned int mms_index = 0;
static double mms_length_counter = 0.0;

void mms__skipwhite_and_nums_s()
{
    int c;
    while(  (c = mms_buf[mms_index++]) == ' ' 
            || c == '\n'
            || c == '\t'
            || (c < 58 && c > 47)   )
        ;                       /* discard whitespace and spurious numbers */
    mms_index--;
}

void mms__skipwhite_s()
{
    int c;
    while(  (c = mms_buf[mms_index++]) == ' ' 
            || c == '\n'
            || c == '\t'   )
        ;                       /* discard whitespace */
    mms_index--;
}

void mms__skipline_s()
{
    int c;
    while(  (c = mms_buf[mms_index++]) != '\n'
            && c != NULLCHAR     )
        ;                       /* discard current line */
    if( c == NULLCHAR ) mms_index--;
}


int mms__get_token_s()
{
    int c;
    mms__skipwhite_and_nums_s();
    while( 1 ) 
    {
        c = mms_buf[mms_index++];
        
        switch( c ) {
            case 'w':   /* pre-read --       */
            case 'a':   /* notes --          */
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
            case ';':   /* track finish     */
            case '/':   /* comment          */
            case NULLCHAR:
                return c;
                break;
            default:
                break;
        }
        mms__skipwhite_and_nums_s();
    }
}


const char* mms__read_file(const char* fn)
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
    
    return (const char*)string;
}





mms_t* mms_open_mem(const char* buf)
{
   int c;
   double rest_len;
   
   mms_buf = buf;
   mms_index = 0;
   
   mms_t* song = (mms_t*)malloc(sizeof(mms_t));
   song->data.beats_per_minute = 140;
   song->data.length = 0.0;
   song->data.measures = NULL;
   mms_length_counter = 0;
   sb_push(song.data.measures
   
}






#pragma GCC diagnostic pop

#endif /* MMS_IMPLEMENTATION */

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
