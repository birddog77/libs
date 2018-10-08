#define DRN_MML_IMPLEMENTATION
#include "../drn_mml.h"

#define DRN_POIS_IMPLEMENTATION
#include "../drn_pois.h"

#include <time.h>

void print_point(POIS_POINT* p)
{
    printf("%f %f\n",p->x,p->y);
}


int main() {

    srand(time(0));
    
    POIS_POINT p;

    int sz = 16;
    float rad = 4.f;

    int num_samps;
    POIS_POINT * data = drn_poisson_plane(&num_samps,sz,rad);

    printf("%d\n",num_samps);
/*
    for( int i=0; i<num_samps; i++ )
    {
        print_point(&data[i]);
        for( int j=0; j<num_samps; j++ )
        {
            if( !(i==j) )
            {
                printf("\t");
                print_point(&data[j]);
                printf("\t\t");
                printf("%f\n",drn__get_dist(&data[i],&data[j]));
            }
        }
    }

    int xind,yind;

    unsigned char * char_data = malloc(sz*sz);
    for( int i=0; i<sz*sz; i++ )
        char_data[i] = ' ';
    for( int i=0; i<num_samps; i++ )
    {
        p = data[i];
        xind = floor(p.x);
        yind = floor(p.y);
        char_data[xind*sz+yind] = '@';
    }

    for(int i=0; i<sz; i++ )
    {
        for( int j=0; j<sz; j++ )
            printf("%c ",char_data[i*sz+j]);
        printf("\n");
    }
*/




    free(data);
    

    return 0;
}
