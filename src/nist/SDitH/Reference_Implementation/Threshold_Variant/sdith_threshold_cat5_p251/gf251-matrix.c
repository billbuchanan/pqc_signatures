#include "gf251.h"
#include <stdlib.h>
#include <string.h>

#include "gf251-internal.h"

/*************************************************/
/***********     MATRIX OPERATIONS    ************/
/*************************************************/

// vz[] += vx[1][] * y[1] + ... + vx[nb][] * y[nb]
void gf251_matcols_muladd(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t size) {
    uint32_t* unreduced_vz = (uint32_t*) calloc(size, sizeof(uint32_t));
    memset(unreduced_vz, 0, sizeof(uint32_t)*size);
    uint64_t ind=0;
    for(uint32_t j=0; j<nb; j++)
        for(uint32_t i=0; i<size; i++)
            unreduced_vz[i] += vx[ind++]*y[j];
    for(uint32_t i=0; i<size; i++)
        vz[i] = (uint8_t)_gf251_reduce32(unreduced_vz[i] + vz[i]);
    free(unreduced_vz);
}

// vz[] += vx[1][] * y[1] + ... + vx[nb][] * y[nb]
void gf251_matcols_muladd_scaled(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t size, uint32_t scale) {
    uint32_t* unreduced_vz = (uint32_t*) calloc(size, sizeof(uint32_t));
    memset(unreduced_vz, 0, sizeof(uint32_t)*size);
    uint64_t ind=0;
    for(uint32_t j=0; j<nb; j++)
        for(uint32_t i=0; i<size; i++)
            unreduced_vz[i] += vx[ind++]*y[scale*j];
    for(uint32_t i=0; i<size; i++)
        vz[scale*i] = (uint8_t)_gf251_reduce32(unreduced_vz[i] + vz[scale*i]);
    free(unreduced_vz);
}

// vz[] += vx[1][] * y[1] + ... + vx[nb][] * y[nb]
// size must be multiple of 16
void gf251_mat16cols_muladd(uint8_t* vz, const uint8_t* y, const uint8_t* vx, uint32_t nb, uint32_t size) {
    gf251_matcols_muladd(vz, y, vx, nb, size);
}
