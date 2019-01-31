
#define DRN_POIS_IMPLEMENTATION
#include "../drn_pois.h"
#include <stdio.h>
#include <time.h>

void print_point(POIS_POINT1* p)
{
    printf("%f \n",p->x);
}


int main(int argc, char **argv) {
    
    int sdnum = time(0);

    if( argc > 1 )
        sdnum = atoi(argv[1]);

    srand(sdnum);
    
    POIS_POINT1 p;
    POIS_POINT1 avg = drn_generate_zero_point1();



    int sz = 100;
    float rad = 2.10f; 

    int num_samps = 12;
    printf("num sampls: %d\n",num_samps);
    POIS_POINT1 * data = (POIS_POINT1*)malloc(sizeof(POIS_POINT1)*num_samps); 
    drn_poisson_line_in_place(data,&num_samps,sz,rad);

    printf("num samps: %d\n",num_samps);

    for( int i=0; i<num_samps; i++ )
    {
        printf("%c ",97+i);
        printf("%f ",floor(data[i].x));
        print_point(&data[i]);
        avg.x += data[i].x;
    }
    avg.x /= (float)num_samps;

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
        char_data[xind] = '@';
    }

    for(int i=0; i<sz; i++ )
    {
        printf("%c ",char_data[i]);
    }

   printf("\n");




    free(data);
    

    return 0;
}
