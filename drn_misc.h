#ifndef __INCLUDED__DRN_MISC_H
#define __INCLUDED__DRN_MISC_H

#ifdef __cplusplus
extern "C" {
#endif

/* some misc useful functions, C89 */
const char* drn_read_file(const char*);
float       drn_randf();
float       drn_gauss(float x, float invar, float mean);
float       drn_gaussRight(float x, float invar, float mean);
float       drn_gaussLeft(float x, float invar, float mean);
float       drn_sign(float);
float       drn_lerp_f(float,float,float);
double      drn_lerp_d(double,double,double);
int         drn_mod(int,int);   /* works with negative numbers */
float       drn_modf(float,float);
float       drn_clampf(float,float,float);
float       drn_get_max_range(float **,int,int,int,int);
float       drn_get_min_range(float **,int,int,int,int);
int         drn_fast_floor(float);


#ifdef __cplusplus
}
#endif

#endif /* __INCLUDED__DRN_MISC_H */


/* ////////////////////////////////////////////////////////////////////
 * 
 * 
 * 
                IMPLEMENTATION                  ///////////////////////
                * 
                * 
                * 
//////////////////////////////////////////////////////////////////// */
#ifdef DRN_MISC_IMPLEMENTATION


#include "drn_misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

const char* drn_read_file(const char* fn)
{
    FILE* f = fopen(fn, "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    rewind(f);

    char* string = malloc(fsize + 1);
    fread(string, fsize, 1, f);
    fclose(f);

    string[fsize] = 0;
    
    return (const char*)string;
}

float drn_randf()
{
    return ((float)rand())/((float)RAND_MAX);
}


float drn_gauss(float x, float invar, float mean)
{
    return exp(-pow(x-mean,2.f)*0.5f*invar*invar);
}

/* gauss distribution bounded on the right */
float drn_gaussRight(float x, float invar, float mean)
{
    if( x > mean ) x = mean;
    return exp(-pow(x-mean,2.f)*0.5f*invar*invar);
}

/* gauss distribution bounded on the left */
float drn_gaussLeft(float x, float invar, float mean)
{
    if( x < mean ) x = mean;
    return exp(-pow(x-mean,2.f)*0.5f*invar*invar);
}

float drn_sign(float f)
{
    return f < 0.f ? -1.f : 1.f;
}

float drn_lerp_f(float x,float y,float alpha)
{
    return x * (1 - alpha) + alpha * y;
}
double drn_lerp_d(double x,double y,double alpha)
{
    return x*(1-alpha) + alpha*y;
}

/*
 * C++ operator '%' returns remainder, thus does not work with negative values
 * ( true of C89 ? )
 */
int drn_mod(int k, int n) 
{
    return ((k %= n) < 0) ? k+n : k;
}
    
float drn_modf(float k,float bound)
{
    float r = k/bound;
    return (r - (float)floor(r)) * bound;
}

float drn_clampf(float v,float min,float max)
{
    if( v < min )
        v = min;
    else if( v > max )
        v = max;
    return v;
}

/*
 * Returns the maximum value found in the range [xmin,xmax), [ymin,ymax)
 * of a 2D array of normalized floats
 */
float drn_get_max_range(float ** data,int xmin,int xmax,int ymin,int ymax)
{
    int i,j;
    float result = 0.0f;
    
    for( i=ymin; i<ymax; ++i )
        for( j=xmin; j<xmax; ++j )
        {
            if( data[i][j] > result )
                result = data[i][j];
        }
    
    return result;
}

/*
 * Returns the minimum value found in the range [xmin,xmax), [ymin,ymax)
 * of a 2D array of normalized floats
 */
float drn_get_min_range(float ** data,int xmin,int xmax,int ymin,int ymax)
{
    int i,j;
    float result = 1.0f;
    
    for( i=ymin; i<ymax; ++i )
        for( j=xmin; j<xmax; ++j )
        {
            if( data[i][j] < result )
                result = data[i][j];
        }
    
    return result;
}

int drn_fast_floor(float f)
{
    return f > 0 ? (int)f : (int)f-1;
}

#endif // DRN_MISC_IMPLEMENTATION
