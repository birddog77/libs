#define DRN_MML_IMPLEMENTATION
#include "../drn_mml.h"

#define DRN_POIS_IMPLEMENTATION
#include "../drn_pois.h"

#include <time.h>



int main() {

    srand(time(0));

    printf("%f\n",POIS_RAND());

    
    return 0;
}
