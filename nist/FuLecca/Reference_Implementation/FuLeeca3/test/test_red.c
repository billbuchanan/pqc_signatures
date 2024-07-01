#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../params.h"
#include "../coeff.h"

int main(void) {

    coeff tmp;
    d_coeff c;

    for (size_t i=0; i<P; i++) {
        for (size_t j=0; j<P; j++) {
            c = i * j;
            tmp = coeff_red_modp(c);
            printf("%d\n", tmp);
        }
    }
  return 0;
}
