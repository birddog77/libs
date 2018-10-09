#define DRN_MML_IMPLEMENTATION
#include "../drn_mml.h"

#define DRN_POIS_IMPLEMENTATION
#include "../drn_pois.h"

#include <time.h>

void print_point(POIS_POINT* p)
{
    printf("%f %f\n",p->x,p->y);
}


int main(int argc, char **argv) {
    
    int sdnum = time(0);

    if( argc > 1 )
        sdnum = atoi(argv[1]);

    srand(sdnum);
    
    POIS_POINT p;
    POIS_POINT avg = drn_generate_zero_point();

    int sz = 16;
    float rad = 3.2f; 

    int num_samps;
    POIS_POINT * data = drn_poisson_disk(&num_samps,sz/2,rad);

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
        xind = floor(p.x+sz/2);
        yind = floor(p.y+sz/2);
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
