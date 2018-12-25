#ifndef __INCLUDED__DRN_BEZIER_H__
#define __INCLUDED__DRN_BEZIER_H__

#ifdef __cplusplus
extern "C" {
#endif

struct BEZ_POINT2D {
   float x;
   float y;
};

float drn_bezier_cubic2D(float t, BEZ_POINT2D control[4]);



#ifdef __cplusplus
}
#endif

#endif // __INCLUDED__DRN_BEZIER_H__



// ***************************** IMPLEMENTATION *******************************
#ifdef DRN_BEZIER_IMPLEMENTATION

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#include <math.h>

BEZ_POINT2D drn_bezier_cubic2D(float t, BEZ_POINT2D w[4])
{
   BEZ_POINT2D res;
   float t2 = t*t;
   float t3 = t2*t;
   float mt = 1.f - t;
   float mt2 = mt*mt;
   float mt3 = mt2*mt;
   res.x = w[0].x*mt3 + 3*w[1].x*mt2*t + 3*w[2].x*mt*t2 + w[3].x*t3;
   res.y = w[0].y*mt3 + 3*w[1].y*mt2*t + 3*w[2].y*mt*t2 + w[3].y*t3;
   return res;
}





#pragma GCC diagnostic pop

#endif /* DRN_BEZIER_IMPLEMENTATION */
