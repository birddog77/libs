
#ifndef __INCLUDED__DRN_POIS_H__
#define __INCLUDED__DRN_POIS_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x;
    float y;
} POIS_POINT;

/*
 * Generates a uniform random point normalized over the input space
 * */
POIS_POINT drn_generate_uniform_point(int space);

/*
 * Generates a zero (0,0) point
 * */
POIS_POINT drn_generate_zero_point();

/*
 * Generates Poisson-distributed points on a plane of the given size
 *  - the number of samples generated is returned in num_samples
 * */
POIS_POINT * drn_poisson_plane( int * num_samples,
                                int space_size,
                                float separation            );
void drn_poisson_plane_in_place(POIS_POINT * data,
                                int * num_samples,
                                int space_size,
                                float separation            );
/*
 * Generates Poisson-distributed points on a zero-centered disk
 *  - similar to plane, but restricts points based on a radius
 * */
POIS_POINT * drn_poisson_disk(  int * num_samples,
                                int space_radius,
                                float separation            );
void drn_poisson_disk_in_place( POIS_POINT * data,
                                int * num_samples,
                                int space_radius,
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

float drn__get_dist(POIS_POINT* p0, POIS_POINT* p1)
{
    float dist;
    dist = sqrtf(powf(p1->x - p0->x,2.f) + powf(p1->y - p0->y,2.f));
    return dist;
}

int drn__check_bounds_plane(POIS_POINT p,int space_size)
{
    int res;
    res =   (p.x > 0.f && p.x < space_size) &&
            (p.y > 0.f && p.y < space_size);
    return res;
}

int drn__check_bounds_disk(POIS_POINT p,int radius)
{
    POIS_POINT zp = drn_generate_zero_point();
    float dist = drn__get_dist(&p,&zp);
    
    int res = (dist < radius);
    return res;
}


int drn__check_bg_grid( POIS_POINT p,
                        POIS_POINT* points,
                        int* bg_grid,
                        int grid_size,
                        float unit_size,
                        float radius        )
{
    int tmp = (int)(floor(p.y/unit_size)*grid_size+floor(p.x/unit_size));
    if( bg_grid[tmp] >= 0 )
    {
        return 0;
    }
    
    int startx = drn__clamp((int)(floor(p.x/unit_size)-2),0,grid_size-1);
    int starty = drn__clamp((int)(floor(p.y/unit_size)-2),0,grid_size-1);
    int endx = drn__clamp((int)(ceil(p.x/unit_size)+2),0,grid_size-1);
    int endy = drn__clamp((int)(ceil(p.y/unit_size)+2),0,grid_size-1);
    int ind;
    float dist;
    
    POIS_POINT p1;
    
    for( int y=starty; y<=endy; ++y ) {
        for( int x=startx; x<=endx; ++x )
        {
            ind = drn__clamp(y*grid_size+x,0,grid_size*grid_size-1);

            if( bg_grid[ind] >= 0 )
            {
                p1 = points[bg_grid[ind]];
                dist = drn__get_dist(&p1,&p);
                if( dist < radius )
                {
                    return 0;
                }
            }
        }    
    }
    
    return 1;
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
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT p,pk;
    
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
    int tmp = (int)(floor(p.y/unit_sz)*grid_dim+floor(p.x/unit_sz));
    
    bg_grid[tmp] = num_points-1;
    
    // generate points
    while( num_active > 0 )
    {
        active_ind = floor(POIS_RAND() * num_active);
        p = point_list[active_list[active_ind]];
        c = 0;
        
        while(1)
        {
            c += 1;
            pk = drn__generate_radial_point(p,separation);
            
            if( drn__check_bounds_plane(pk,space_size) &&
                drn__check_bg_grid(pk,point_list,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                point_list[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.y/unit_sz)*grid_dim+floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == DRN_k )
            {   // remove p from active list
                active_list[active_ind] = active_list[--num_active];
                break;
            }
        }
    }
    
    
    // free background grid
    free(bg_grid);
    free(active_list);

    *num_samples = num_points;

    return point_list;
}

void drn_poisson_plane_in_place( POIS_POINT * data,
                                    int * num_samples,
                                    int space_size,
                                    float separation            )
{
    int max_samples = (*num_samples);

    int failout = 0;
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * sin(DRN_PI_quarter);
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = malloc(sizeof(int)*grid_dim*grid_dim);
    unsigned int * active_list = malloc(sizeof(unsigned int)*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    //~ POIS_POINT * point_list = malloc(sizeof(POIS_POINT)*grid_dim*grid_dim);
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = drn_generate_uniform_point(space_size);
    data[num_points++] = p;
    active_list[num_active++] = num_points-1;
    int tmp = (int)(floor(p.y/unit_sz)*grid_dim+floor(p.x/unit_sz));
    
    bg_grid[tmp] = num_points-1;
    
    // generate points
    while( num_active > 0 && num_points < max_samples )
    {
        active_ind = floor(POIS_RAND() * num_active);
        p = data[active_list[active_ind]];
        c = 0;
        
        while(1)
        {
            c += 1;
            pk = drn__generate_radial_point(p,separation);
            
            if( drn__check_bounds_plane(pk,space_size) &&
                drn__check_bg_grid(pk,data,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                data[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.y/unit_sz)*grid_dim+floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == DRN_k )
            {   // remove p from active list
                active_list[active_ind] = active_list[--num_active];
                break;
            }
        }
    }
    
    // free background grid
    free(bg_grid);
    free(active_list);

    *num_samples = num_points;
}

POIS_POINT * drn_poisson_disk(  int * num_samples,
                                int space_radius,
                                float separation            )
{
    int space_size = 2*space_radius;
    POIS_POINT cp;
    cp.x = space_radius;
    cp.y = space_radius;
    
    int failout = 0;
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * sin(DRN_PI_quarter);
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT p,pk;
    
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
    while( drn__get_dist(&p,&cp) > space_radius )
        p = drn_generate_uniform_point(space_size);
    point_list[num_points++] = p;
    active_list[num_active++] = num_points-1;
    int tmp = (int)(floor(p.y/unit_sz)*grid_dim+floor(p.x/unit_sz));
    
    bg_grid[tmp] = num_points-1;
    
    // generate points
    while( num_active > 0 )
    {
        active_ind = floor(POIS_RAND() * num_active);
        p = point_list[active_list[active_ind]];
        c = 0;
        
        while(1)
        {
            c += 1;
            pk = drn__generate_radial_point(p,separation);
            
            if( ( drn__get_dist(&pk,&cp) < space_radius ) &&
                drn__check_bg_grid(pk,point_list,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                point_list[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.y/unit_sz)*grid_dim+floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == DRN_k )
            {   // remove p from active list
                active_list[active_ind] = active_list[--num_active];
                break;
            }
        }
    }
    
    // free background grid
    free(bg_grid);
    free(active_list);
    
    // shift points to zero-centered
    for( int i=0; i<num_points; ++i )
    {
        point_list[i].x -= space_radius;
        point_list[i].y -= space_radius;
    }

    *num_samples = num_points;

    return point_list;
}

void drn_poisson_disk_in_place( POIS_POINT * data,
                                int * num_samples,
                                int space_radius,
                                float separation            )
{
    int max_samples = (*num_samples);

    int space_size = 2*space_radius;
    POIS_POINT cp;
    cp.x = space_radius;
    cp.y = space_radius;
    
    int failout = 0;
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * sin(DRN_PI_quarter);
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = malloc(sizeof(int)*grid_dim*grid_dim);
    unsigned int * active_list = malloc(sizeof(unsigned int)*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    //~ POIS_POINT * point_list = malloc(sizeof(POIS_POINT)*grid_dim*grid_dim);
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = drn_generate_uniform_point(space_size);
    while( drn__get_dist(&p,&cp) > space_radius )
        p = drn_generate_uniform_point(space_size);
    data[num_points++] = p;
    active_list[num_active++] = num_points-1;
    int tmp = (int)(floor(p.y/unit_sz)*grid_dim+floor(p.x/unit_sz));
    
    bg_grid[tmp] = num_points-1;
    
    // generate points
    while( num_active > 0 && num_points < max_samples )
    {
        active_ind = floor(POIS_RAND() * num_active);
        p = data[active_list[active_ind]];
        c = 0;
        
        while(1)
        {
            c += 1;
            pk = drn__generate_radial_point(p,separation);
            
            if( ( drn__get_dist(&pk,&cp) < space_radius ) &&
                drn__check_bg_grid(pk,data,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                data[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.y/unit_sz)*grid_dim+floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == DRN_k )
            {   // remove p from active list
                active_list[active_ind] = active_list[--num_active];
                break;
            }
        }
    }
    
    // free background grid
    free(bg_grid);
    free(active_list);
    
    // shift points to zero-centered
    for( int i=0; i<num_points; ++i )
    {
        data[i].x -= space_radius;
        data[i].y -= space_radius;
    }

    *num_samples = num_points;
}


#pragma GCC diagnostic pop

#endif /* DRN_POIS_IMPLEMENTATION */
