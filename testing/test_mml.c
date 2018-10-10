#define DRN_MML_IMPLEMENTATION
#include "../drn_mml.h"

#define DRN_POIS_IMPLEMENTATION
#include "../drn_pois.h"

#include <time.h>

void print_point(POIS_POINT2* p)
{
    printf("%f %f\n",p->x,p->y);
}


int main(int argc, char **argv) {
    
    int sdnum = time(0);

    if( argc > 1 )
        sdnum = atoi(argv[1]);

    srand(sdnum);
    
    POIS_POINT2 p;
    POIS_POINT2 avg = drn_generate_zero_point2();



    int sz = 32;
    float rad = 3.2f; 

    int num_samps = 80;
    POIS_POINT2 * data = malloc(sizeof(POIS_POINT2)*num_samps);
    drn_poisson_plane_in_place(data,&num_samps,sz,rad);

    printf("num samps: %d\n",num_samps);

    for( int i=0; i<num_samps; i++ )
    {
        printf("%c ",97+i);
        print_point(&data[i]);
        avg.x += data[i].x;
        avg.y += data[i].y;
    }
    avg.x /= (float)num_samps;
    avg.y /= (float)num_samps;

    printf("avg: ");
    print_point(&avg);

    int xind,yind;

    unsigned char * char_data = malloc(sz*sz);
    for( int i=0; i<sz*sz; i++ )
        char_data[i] = ' ';
    for( int i=0; i<num_samps; i++ )
    {
        p = data[i];
        xind = floor(p.x);
        yind = floor(p.y);
        char_data[yind*sz+xind] = '@';
    }

    for(int i=0; i<sz; i++ )
    {
        for( int j=0; j<sz; j++ )
            printf("%c ",char_data[i*sz+j]);
        printf("\n");
    }





    free(data);
    

    return 0;
}
