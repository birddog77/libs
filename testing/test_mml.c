
#define DRN_MML_IMPLEMENTATION
#include "../drn_mml.h"
#include <stdio.h>
#include <time.h>



int main(int argc, char **argv) {
    drn_mml_t * dt = drn_mml_open_file("mysong.mml");

   printf("%f\n",dt->data.length);
   
   int i;
   for( i=0; i<20; i++ )
   {
      double d = drn_mml_decode_stream(dt,i*0.00005);
      printf("%f\n",d);
      
   }
    return 0;
}
