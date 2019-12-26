
#ifndef __INCLUDED__POIS_H__
#define __INCLUDED__POIS_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x;
} POIS_POINT1;

typedef struct {
    float x;
    float y;
} POIS_POINT2;

typedef struct {
    float x;
    float y;
    float z;
} POIS_POINT3;

/*
 * Generates a uniform random point normalized over the input space
 * */
POIS_POINT1 generate_uniform_point1(int space);
POIS_POINT2 generate_uniform_point2(int space);
POIS_POINT3 generate_uniform_point3(int space);

/*
 * Generates a zero (0,0) point
 * */
POIS_POINT1 generate_zero_point1();
POIS_POINT2 generate_zero_point2();
POIS_POINT3 generate_zero_point3();



/*
 * Generates Poisson-distributed points on a line of the given size
 *  - the number of samples generated is returned in num_samples
 *  - for in-place version, the size of the buffer should be input in
 *    num_samples
 * */
POIS_POINT1 * poisson_line(    int * num_samples,
                                    int space_size,
                                    float separation            );
void poisson_line_in_place(    POIS_POINT1 * data,
                                    int * num_samples,
                                    int space_size,
                                    float separation            );


/*
 * Generates Poisson-distributed points on a plane of the given size
 *  - the number of samples generated is returned in num_samples
 *  - for in-place version, the size of the buffer should be input in
 *    num_samples
 * */
POIS_POINT2 * poisson_plane(    int * num_samples,
                                    int space_size,
                                    float separation            );
void poisson_plane_in_place(    POIS_POINT2 * data,
                                    int * num_samples,
                                    int space_size,
                                    float separation            );
/*
 * Generates Poisson-distributed points on a zero-centered disk
 *  - similar to plane, but restricts points based on a radius
 *  - the number of samples generated is returned in num_samples
 *  - for in-place version, the size of the buffer should be input in
 *    num_samples
 * */
POIS_POINT2 * poisson_disk( int * num_samples,
                                int space_radius,
                                float separation            );
void poisson_disk_in_place( POIS_POINT2 * data,
                                int * num_samples,
                                int space_radius,
                                float separation            );

/*
 * Generates Poisson-distributed points in a cube of the given size
 *  - 3D version of poisson_plane
 *  - the number of samples generated is returned in num_samples
 *  - for in-place version, the size of the buffer should be input in
 *    num_samples
 * */
POIS_POINT3 * poisson_box(  int * num_samples,
                                int space_size,
                                float separation            );
void poisson_box_in_place(  POIS_POINT3 * data,
                                int * num_samples,
                                int space_size,
                                float separation            );

/*
 * Generates Poisson-distributed points in a zero-centered sphere
 *  - 3D version of poisson_disk
 *  - the number of samples generated is returned in num_samples
 *  - for in-place version, the size of the buffer should be input in
 *    num_samples
 * */
POIS_POINT3 * poisson_sphere(   int * num_samples,
                                    int space_size,
                                    float separation            );
void poisson_sphere_in_place(   POIS_POINT3 * data,
                                    int * num_samples,
                                    int space_size,
                                    float separation            );

#ifdef __cplusplus
}
#endif


#endif /* __INCLUDED__POIS_H__ */



/*                                  ********************
 *          IMPLEMENTATION          ********************
 *                                  ********************
 * */
#ifdef POIS_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <math.h>

/* can override this macro for other sources of uniform random variables */
#ifndef POIS_RAND
#include <stdlib.h>
#define POIS_RAND() ((float)rand()/(float)RAND_MAX)
#endif

#define POIS_PI                3.14159265359
#define POIS_PI_quarter        0.7853981634
#define POIS_PI_quarter_sine   0.7071067812
#define POIS_k 30


POIS_POINT1 generate_uniform_point1(int space_size)
{
    POIS_POINT1 p;
    p.x = POIS_RAND() * space_size;
    return p;
}

POIS_POINT2 generate_uniform_point2(int space_size)
{
    POIS_POINT2 p;
    p.x = POIS_RAND() * space_size;
    p.y = POIS_RAND() * space_size;
    return p;
}

POIS_POINT3 generate_uniform_point3(int space_size)
{
    POIS_POINT3 p;
    p.x = POIS_RAND() * space_size;
    p.y = POIS_RAND() * space_size;
    p.z = POIS_RAND() * space_size;
    return p;
}

POIS_POINT1 generate_zero_point1()
{
    POIS_POINT1 p;
    p.x = 0.f;
    return p;
}

POIS_POINT2 generate_zero_point2()
{
    POIS_POINT2 p;
    p.x = 0.f;
    p.y = 0.f;
    return p;
}

POIS_POINT3 generate_zero_point3()
{
    POIS_POINT3 p;
    p.x = 0.f;
    p.y = 0.f;
    p.z = 0.f;
    return p;
}

POIS_POINT1 pois__generate_radial_point1(POIS_POINT1 p,float r)
{
   float f = POIS_RAND()*2.f - 1.f;
   float radius = r*f;
   radius += ( f > 0.f ? r : -r );

   POIS_POINT1 result;
   result.x = p.x+radius;
   return result;
}

POIS_POINT2 pois__generate_radial_point2(POIS_POINT2 p,float r)
{
   float theta = POIS_RAND() * POIS_PI * 2.f;
   float radius = r + POIS_RAND()*r;

   float x_run = radius * cos(theta);
   float y_run = radius * sin(theta);

   POIS_POINT2 result;
   result.x = p.x+x_run;
   result.y = p.y+y_run;
   return result;
}

POIS_POINT3 pois__generate_radial_point3(POIS_POINT3 p,float r)
{
   float phi = POIS_RAND() * POIS_PI;
   float theta = POIS_RAND() * POIS_PI * 2.f;
   float radius = r + POIS_RAND()*r;

   float x_run = radius * sin(theta) * cos(phi);
   float y_run = radius * sin(theta) * sin(phi);
   float z_run = radius * cos(theta);
     
   POIS_POINT3 result;
   result.x = p.x+x_run;
   result.y = p.y+y_run;
   result.z = p.z+z_run;
   return result;
}

int pois__clamp(int x,int min,int max) 
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

float pois__get_dist1(POIS_POINT1* p0, POIS_POINT1* p1)
{
   float dist;
   dist = abs(p1->x - p0->x);
   return dist;
}

float pois__get_dist2(POIS_POINT2* p0, POIS_POINT2* p1)
{
   float dist;
   dist = sqrtf(  powf(p1->x - p0->x,2.f) +
                  powf(p1->y - p0->y,2.f)     );
   return dist;
}

float pois__get_dist3(POIS_POINT3* p0, POIS_POINT3* p1)
{
   float dist;
   dist = sqrtf(  powf(p1->x - p0->x,2.f) +
                  powf(p1->y - p0->y,2.f) +
                  powf(p1->z - p0->z,2.f)     );
   return dist;
}

int pois__check_bounds_line(POIS_POINT1 p,int space_size)
{
   int res;
   res =   (p.x > 0.f && p.x < space_size);
   return res;
}

int pois__check_bounds_plane(POIS_POINT2 p,int space_size)
{
   int res;
   res =    (p.x > 0.f && p.x < space_size) &&
            (p.y > 0.f && p.y < space_size);
   return res;
}

int pois__check_bounds_box(POIS_POINT3 p,int space_size)
{
   int res;
   res =    (p.x > 0.f && p.x < space_size) &&
            (p.y > 0.f && p.y < space_size) &&
            (p.z > 0.f && p.z < space_size);
   return res;
}

int pois__check_bounds_disk(POIS_POINT2 p,int radius)
{
   POIS_POINT2 zp = generate_zero_point2();
   float dist = pois__get_dist2(&p,&zp);

   int res = (dist < radius);
   return res;
}

int pois__check_bounds_sphere(POIS_POINT3 p,int radius)
{
   POIS_POINT3 zp = generate_zero_point3();
   float dist = pois__get_dist3(&p,&zp);

   int res = (dist < radius);
   return res;
}

int pois__check_bg_grid3(   POIS_POINT3 p,
                           POIS_POINT3* points,
                           int* bg_grid,
                           int grid_size,
                           float unit_size,
                           float radius         )
{
    int tmp = (int)(floor(p.z/unit_size)*grid_size*grid_size +
                    floor(p.y/unit_size)*grid_size +
                    floor(p.x/unit_size));
    if( bg_grid[tmp] >= 0 )
    {
        return 0;
    }
    
    int startx = pois__clamp((int)(floor(p.x/unit_size)-2),0,grid_size-1);
    int starty = pois__clamp((int)(floor(p.y/unit_size)-2),0,grid_size-1);
    int startz = pois__clamp((int)(floor(p.z/unit_size)-2),0,grid_size-1);
    int endx = pois__clamp((int)(ceil(p.x/unit_size)+2),0,grid_size-1);
    int endy = pois__clamp((int)(ceil(p.y/unit_size)+2),0,grid_size-1);
    int endz = pois__clamp((int)(ceil(p.z/unit_size)+2),0,grid_size-1);
    int ind;
    float dist;
    
    POIS_POINT3 p1;
    
    for( int z=startz; z<=endz; ++z ) {
        for( int y=starty; y<=endy; ++y ) {
            for( int x=startx; x<=endx; ++x )
            {
                ind = pois__clamp(   z*grid_size*grid_size+
                                    y*grid_size+x,
                                    0,grid_size*grid_size-1);

                if( bg_grid[ind] >= 0 )
                {
                    p1 = points[bg_grid[ind]];
                    dist = pois__get_dist3(&p1,&p);
                    if( dist < radius )
                    {
                        return 0;
                    }
                }
            }    
        }
    }
    
    return 1;
}


int pois__check_bg_grid2(    POIS_POINT2 p,
                            POIS_POINT2* points,
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
    
    int startx = pois__clamp((int)(floor(p.x/unit_size)-2),0,grid_size-1);
    int starty = pois__clamp((int)(floor(p.y/unit_size)-2),0,grid_size-1);
    int endx = pois__clamp((int)(ceil(p.x/unit_size)+2),0,grid_size-1);
    int endy = pois__clamp((int)(ceil(p.y/unit_size)+2),0,grid_size-1);
    int ind;
    float dist;
    
    POIS_POINT2 p1;
    
    for( int y=starty; y<=endy; ++y ) {
        for( int x=startx; x<=endx; ++x )
        {
            ind = pois__clamp(y*grid_size+x,0,grid_size*grid_size-1);

            if( bg_grid[ind] >= 0 )
            {
                p1 = points[bg_grid[ind]];
                dist = pois__get_dist2(&p1,&p);
                if( dist < radius )
                {
                    return 0;
                }
            }
        }    
    }
    
    return 1;
}

int pois__check_bg_grid1(    POIS_POINT1 p,
                            POIS_POINT1* points,
                            int* bg_grid,
                            int grid_size,
                            float unit_size,
                            float radius        )
{
   int tmp = (int)(floor(p.x/unit_size));
   if( bg_grid[tmp] >= 0 )
   {
      return 0;
   }

   int startx = pois__clamp((int)(floor(p.x/unit_size)-2),0,grid_size-1);
   int endx = pois__clamp((int)(ceil(p.x/unit_size)+2),0,grid_size-1);
   int ind;
   float dist;

   POIS_POINT1 p1;
    
   for( int x=startx; x<=endx; ++x )
   {
      ind = pois__clamp(x,0,grid_size-1);

      if( bg_grid[ind] >= 0 )
      {
         p1 = points[bg_grid[ind]];
         dist = pois__get_dist1(&p1,&p);
         if( dist < radius )
         {
            return 0;
         }
      }
   }    
    
   return 1;
}

void poisson_box_in_place(  POIS_POINT3 * data,
                                int * num_samples,
                                int space_size,
                                float separation            )
{
    int max_samples = (*num_samples);
    
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT3 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim*grid_dim*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point3(space_size);
    data[num_points++] = p;
    active_list[num_active++] = num_points-1;
    int tmp = (int)(    floor(p.z/unit_sz)*grid_dim*grid_dim +
                        floor(p.y/unit_sz)*grid_dim +
                        floor(p.x/unit_sz)  );
    
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
            pk = pois__generate_radial_point3(p,separation);
            
            if( pois__check_bounds_box(pk,space_size) &&
                pois__check_bg_grid3(pk,data,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                data[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(    floor(pk.z/unit_sz)*grid_dim*grid_dim +
                                floor(pk.y/unit_sz)*grid_dim + 
                                floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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


POIS_POINT3 * poisson_box(      int * num_samples,
                                    int space_size,
                                    float separation            )
{
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT3 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim*grid_dim*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    POIS_POINT3 * point_list = (POIS_POINT3*)malloc(sizeof(POIS_POINT2)*grid_dim*grid_dim*grid_dim);
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point3(space_size);
    point_list[num_points++] = p;
    active_list[num_active++] = num_points-1;
    int tmp = (int)(    floor(p.z/unit_sz)*grid_dim*grid_dim +
                        floor(p.y/unit_sz)*grid_dim +
                        floor(p.x/unit_sz)  );
    
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
            pk = pois__generate_radial_point3(p,separation);
            
            if( pois__check_bounds_box(pk,space_size) &&
                pois__check_bg_grid3(pk,point_list,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                point_list[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(    floor(pk.z/unit_sz)*grid_dim*grid_dim +
                                floor(pk.y/unit_sz)*grid_dim + 
                                floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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



POIS_POINT1 * poisson_line(    int * num_samples,
                                    int space_size,
                                    float separation            )
{
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT1 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim);    
    unsigned int num_active = 0;
    POIS_POINT1 * point_list = (POIS_POINT1*)malloc(sizeof(POIS_POINT1)*grid_dim);
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point1(space_size);
    point_list[num_points++] = p;
    active_list[num_active++] = num_points-1;
    int tmp = (int)(floor(p.x/unit_sz));
    
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
            pk = pois__generate_radial_point1(p,separation);
            
            if( pois__check_bounds_line(pk,space_size) &&
                pois__check_bg_grid1(pk,point_list,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                point_list[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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

void poisson_line_in_place(    POIS_POINT1 * data,
                                    int * num_samples,
                                    int space_size,
                                    float separation            )
{
    int max_samples = (*num_samples);

    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT1 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim);    
    unsigned int num_active = 0;
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point1(space_size);
    data[num_points++] = p;
    active_list[num_active++] = num_points-1;
    int tmp = (int)(floor(p.x/unit_sz));
    
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
            pk = pois__generate_radial_point1(p,separation);
            
            if( pois__check_bounds_line(pk,space_size) &&
                pois__check_bg_grid1(pk,data,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                data[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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



POIS_POINT2 * poisson_plane( int * num_samples,
                                 int space_size,
                                 float separation     )
{
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT2 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    POIS_POINT2 * point_list = (POIS_POINT2*)malloc(sizeof(POIS_POINT2)*grid_dim*grid_dim);
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point2(space_size);
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
            pk = pois__generate_radial_point2(p,separation);
            
            if( pois__check_bounds_plane(pk,space_size) &&
                pois__check_bg_grid2(pk,point_list,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                point_list[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.y/unit_sz)*grid_dim+floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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

void poisson_plane_in_place(    POIS_POINT2 * data,
                                    int * num_samples,
                                    int space_size,
                                    float separation            )
{
    int max_samples = (*num_samples);

    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT2 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point2(space_size);
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
            pk = pois__generate_radial_point2(p,separation);
            
            if( pois__check_bounds_plane(pk,space_size) &&
                pois__check_bg_grid2(pk,data,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                data[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.y/unit_sz)*grid_dim+floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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

POIS_POINT2 * poisson_disk(     int * num_samples,
                                    int space_radius,
                                    float separation            )
{
    int space_size = 2*space_radius;
    POIS_POINT2 cp;
    cp.x = space_radius;
    cp.y = space_radius;
    
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT2 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    POIS_POINT2 * point_list = (POIS_POINT2*)malloc(sizeof(POIS_POINT2)*grid_dim*grid_dim);
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point2(space_size);
    while( pois__get_dist2(&p,&cp) > space_radius )
        p = generate_uniform_point2(space_size);
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
            pk = pois__generate_radial_point2(p,separation);
            
            if( ( pois__get_dist2(&pk,&cp) < space_radius ) &&
                pois__check_bg_grid2(pk,point_list,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                point_list[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.y/unit_sz)*grid_dim+floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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

void poisson_disk_in_place( POIS_POINT2 * data,
                                int * num_samples,
                                int space_radius,
                                float separation            )
{
    int max_samples = (*num_samples);

    int space_size = 2*space_radius;
    POIS_POINT2 cp;
    cp.x = space_radius;
    cp.y = space_radius;
    
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT2 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point2(space_size);
    while( pois__get_dist2(&p,&cp) > space_radius )
        p = generate_uniform_point2(space_size);
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
            pk = pois__generate_radial_point2(p,separation);
            
            if( ( pois__get_dist2(&pk,&cp) < space_radius ) &&
                pois__check_bg_grid2(pk,data,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                data[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(floor(pk.y/unit_sz)*grid_dim+floor(pk.x/unit_sz));
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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


POIS_POINT3 * poisson_sphere(   int * num_samples,
                                    int space_radius,
                                    float separation            )
{
    int space_size = 2*space_radius;
    POIS_POINT3 cp;
    cp.x = space_radius;
    cp.y = space_radius;
    cp.z = space_radius;
    
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT3 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim*grid_dim*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    POIS_POINT3 * point_list = (POIS_POINT3*)malloc(sizeof(POIS_POINT2)*grid_dim*grid_dim*grid_dim);
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point3(space_size);
    while( pois__get_dist3(&p,&cp) > space_radius )
        p = generate_uniform_point3(space_size);
    point_list[num_points++] = p;
    active_list[num_active++] = num_points-1;
    int tmp = (int)(    floor(p.z/unit_sz)*grid_dim*grid_dim +
                        floor(p.y/unit_sz)*grid_dim +
                        floor(p.x/unit_sz)      );
    
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
            pk = pois__generate_radial_point3(p,separation);
            
            if( ( pois__get_dist3(&pk,&cp) < space_radius ) &&
                pois__check_bg_grid3(pk,point_list,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                point_list[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(    floor(pk.z/unit_sz)*grid_dim*grid_dim +
                                floor(pk.y/unit_sz)*grid_dim +
                                floor(pk.x/unit_sz)     );
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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
        point_list[i].z -= space_radius;
    }

    *num_samples = num_points;

    return point_list;    
}

void poisson_sphere_in_place(   POIS_POINT3 * data,
                                    int * num_samples,
                                    int space_radius,
                                    float separation            )
{
    int max_samples = (*num_samples);
    
    int space_size = 2*space_radius;
    POIS_POINT3 cp;
    cp.x = space_radius;
    cp.y = space_radius;
    cp.z = space_radius;
    
    int c = 0;
    int active_ind;
    
    // find unit size of a cell
    float unit_sz = separation * POIS_PI_quarter_sine;
    int grid_dim = (int)ceil((float)space_size/unit_sz);
    POIS_POINT3 p,pk;
    
    // allocate background grid and active list for point comparisons
    int * bg_grid = (int*)malloc(sizeof(int)*grid_dim*grid_dim*grid_dim);
    unsigned int * active_list = (unsigned int*)malloc(sizeof(unsigned int)*grid_dim*grid_dim*grid_dim);    
    unsigned int num_active = 0;
    unsigned int num_points = 0;
    
    // initialize background grid with -1
    for( int i=0; i<grid_dim*grid_dim*grid_dim; ++i )
        bg_grid[i] = -1;
        
    // emit initial point
    p = generate_uniform_point3(space_size);
    while( pois__get_dist3(&p,&cp) > space_radius )
        p = generate_uniform_point3(space_size);
    data[num_points++] = p;
    active_list[num_active++] = num_points-1;
    int tmp = (int)(    floor(p.z/unit_sz)*grid_dim*grid_dim +
                        floor(p.y/unit_sz)*grid_dim +
                        floor(p.x/unit_sz)      );
    
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
            pk = pois__generate_radial_point3(p,separation);
            
            if( ( pois__get_dist3(&pk,&cp) < space_radius ) &&
                pois__check_bg_grid3(pk,data,bg_grid,grid_dim,unit_sz,separation) )
            {   // emit this point and add to active list
                data[num_points++] = pk;
                active_list[num_active++] = num_points-1;
                tmp = (int)(    floor(pk.z/unit_sz)*grid_dim*grid_dim +
                                floor(pk.y/unit_sz)*grid_dim +
                                floor(pk.x/unit_sz)     );
                                
                bg_grid[tmp] = num_points-1;
                break;
            }
            else if( c == POIS_k )
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
        data[i].z -= space_radius;
    }

    *num_samples = num_points;
}



#pragma GCC diagnostic pop

#endif /* POIS_IMPLEMENTATION */
