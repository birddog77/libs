
#ifndef __INCLUDED__DRN_POIS_H__
#define __INCLUDED__DRN_POIS_H__


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float x;
    float y;
} POIS_POINT;





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

#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>


/* can override this macro for other sources of uniform random variables */
#ifndef POIS_RAND
#include <stdlib.h>
#define POIS_RAND() ((float)rand()/(float)RAND_MAX)
#endif



#pragma GCC diagnostic pop

#endif /* DRN_POIS_IMPLEMENTATION */


