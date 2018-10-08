
#ifndef __INCLUDED__DRN_POIS_H__
#define __INCLUDED__DRN_POIS_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x;
    float y;
} POIS_POINT;

typedef struct {
    float theta;
    float r;
} SPHERE_POINT;


POIS_POINT drn_generate_uniform_point(int);
POIS_POINT drn_generate_zero_point();

POIS_POINT * drn_poisson_plane( int * num_samples,
                                int space_size,
                                float separation            );

POIS_POINT * drn_poisson_disk(  int * num_samples,
                                int space_radius,
                                POIS_POINT center,
                                float separation            );




#ifdef __cplusplus
}
#endif


#endif /* __INCLUDED__DRN_POIS_H__ */



/*                                  ********************
 *          IMPLEMENTATION          ********************
 *                                  ********************
 * */
#ifdef DRN_POIS_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <math.h>
#include <assert.h>

/* can override this macro for other sources of uniform random variables */
#ifndef POIS_RAND
#include <stdlib.h>
#define POIS_RAND() ((float)rand()/(float)RAND_MAX)
#endif

#define DRN_PI 3.14159265359
#define DRN_PI_quarter 0.7853981634
#define DRN_k 30


POIS_POINT drn_generate_uniform_point(int space_size)
{
    POIS_POINT p;
    p.x = POIS_RAND() * space_size;
    p.y = POIS_RAND() * space_size;
    return p;
}

POIS_POINT drn_generate_zero_point()
{
    POIS_POINT p;
    p.x = 0.f;
    p.y = 0.f;
    return p;
}

POIS_POINT drn__generate_radial_point(POIS_POINT p,float r)
{
    float theta = POIS_RAND() * DRN_PI * 2.f;
    float radius = r + POIS_RAND()*r;
    float x_run = radius * cos(theta);
    float y_run = radius * sin(theta);
    POIS_POINT result;
    result.x = p.x+x_run;
    result.y = p.y+y_run;
    return result;
}

int drn__clamp(int x,int min,int max) 
{
    int res;
    if( x < min )
        res = min;
    else if( x > max )
        res = max;
    else
        res = x;
    return res;
}

int drn__check_bounds_plane(POIS_POINT p,int space_size)
{
    int res;
    res =   (p.x > 0.f && p.x < space_size) &&
            (p.y > 0.f && p.y < space_size);
    return res;
}

float drn__get_dist(POIS_POINT* p0, POIS_POINT* p1)
{
    float dist;
    dist = sqrt(powf(p1->x - p0->x,2.f) + powf(p1->y - p0->y,2.f));
    return dist;
}

int drn__check_bg_grid( POIS_POINT p,
                        POIS_POINT* points,
                        int* bg_grid,
                        int grid_size,
                        float unit_size,
                        float radius        )
{
    int res = 1;
    
    int startx = drn__clamp(floor(floor(p.x-radius) / unit_size),0,grid_size);
    int starty = drn__clamp(floor(floor(p.y-radius) / unit_size),0,grid_size);
    int endx = drn__clamp(ceil(ceil(p.x+radius) / unit_size),0,grid_size);
    int endy = drn__clamp(ceil(ceil(p.y+radius) / unit_size),0,grid_size);
    int ind;
    
    POIS_POINT p1;
    
    for( int y=starty; y<=endy; ++y )
        for( int x=startx; x<=endx; ++x )
        {
            ind = y*grid_size+x;
            if( bg_grid[ind] >= 0 )
            {
                p1 = points[bg_grid[ind]];
                if( drn__get_dist(&p1,&p) < radius )
                {
                    res = 0;
                    break;
                }
            }
        }
    
    return res;
}


POIS_POINT * drn_poisson_plane( int * num_samples,
                                int space_size,
                                float separation            )
{
    int failout = 0;
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * sin(DRN_PI_quarter);
    int grid_dim = ceil((float)space_size/unit_sz);
    POIS_POINT p,pk;
    
    printf("unit_sz: %f\n",unit_sz);
    printf("grid_dim: %d\n",grid_dim);

    // allocate background grid and active list for point comparisons
    int * bg_grid = malloc(sizeof(int)*grid_dim*grid_dim);
    unsigned int * active_list = malloc(sizeof(unsigned int)*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    POIS_POINT * point_list = malloc(sizeof(POIS_POINT)*grid_dim*grid_dim);
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
    
    // emit initial point
    p = drn_generate_uniform_point(space_size);
    point_list[num_points++] = p;
    active_list[num_active++] = num_points-1;
    bg_grid[(int)(floor(p.y/unit_sz)*grid_dim+floor(p.x/unit_sz))] 
        = num_points-1;
    
    
    // generate points
    active_ind = floor(POIS_RAND() * num_active);
    pk = drn__generate_radial_point(p,separation);
    while(1)
    {
        printf("%d\n",c);
        c++;
        pk = drn__generate_radial_point(p,separation);
        if(c >= 30)
        {
            printf("c is too high\n");
            break;
        }
        else if( !drn__check_bounds_plane(pk,space_size) )
        {
            printf("failed bounds check\n");
            break;
        }
        else if( !drn__check_bg_grid(pk,point_list,bg_grid,grid_dim,unit_sz,separation) )
        {
            printf("failed grid check\n");
            break;
        }
        //~ else
        //~ {
            //~ printf("there was no fail\n");
            //~ break;
        //~ }
        
    }
    printf("is it in bounds: %d\n",drn__check_bounds_plane(pk,space_size));
    printf("iterations: %d\npoint: %f %f\n",c,pk.x,pk.y);
    printf("origin point: %f %f\n",p.x,p.y);
    printf("origin grid point: %i %i\n",floor(p.x/unit_sz),floor(p.y/unit_sz));
    printf("dist: %f\n",drn__get_dist(&p,&pk));
    
    // free background grid
    free(bg_grid);
    free(active_list);

    *num_samples = num_points;

    return point_list;
}

POIS_POINT * drn_poisson_disk(  int * num_samples,
                                int space_radius,
                                POIS_POINT center,
                                float separation            )
{
}








#pragma GCC diagnostic pop

#endif /* DRN_POIS_IMPLEMENTATION */


