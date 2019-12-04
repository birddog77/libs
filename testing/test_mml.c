
#define DRN_MML_IMPLEMENTATION
#include "../drn_mml.h"
#include <stdio.h>
#include <time.h>



int main(int argc, char **argv) {
    drn_mml_t * dt = drn_mml_open_file("mysong.mml");

   printf("%f\n",dt->data.length);

   
    return 0;
}
