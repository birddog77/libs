#define DRN_MML_IMPLEMENTATION
#include "../drn_mml.h"


int main() {
    
    int n = 0;
    double d;
    while( n < 500 )
    {
        d = n*(0.1234);
        printf("%f\n",DRN_SIN(DRN_PI*DRN_MODF(DRN_PI_inv*d)));
        n++;
    }
    
    return 0;
}
