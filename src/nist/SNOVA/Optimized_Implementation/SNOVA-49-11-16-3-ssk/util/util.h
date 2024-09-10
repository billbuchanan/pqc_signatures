#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdio.h>

#include "../gf16.h"
#include "../gf16_matrix.h"

#ifdef __cplusplus
extern "C" {
#endif

void print_gf16(gf16_t* data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        printf("%02X-", data[i]);
    }
    printf("\n");
}

void print_byte(uint8_t* data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

void print_gf16m(gf16m_t data) {
    for (uint32_t i = 0; i < rank; i++) {
        for (uint32_t j = 0; j < rank; j++) {
            if (j == 0) {
                if (i == 0) {
                    printf("┌ ");
                } else if (i == rank - 1) {
                    printf("└ ");
                } else {
                    printf("| ");
                }
            }
            printf("%02X ", get_gf16m(data, i, j));
            if (j == rank - 1) {
                if (i == 0) {
                    printf("┐");
                } else if (i == rank - 1) {
                    printf("┘");
                } else {
                    printf("|");
                }
            }
        }
        printf("\n");
    }
}

#ifdef __cplusplus
}
#endif

#endif
