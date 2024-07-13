#include "gf251.h"
#include <stdlib.h>
#include <string.h>

#include "gf251-internal.h"

/*************************************************/
/***********      FIELD EXTENSION      ***********/
/*************************************************/

/* Optimized implementation of GF(251^4)
 *   Tower implementation of GF(251^4):
 *         GF(251) -> GF(251^2): X^2 - 2 = 0
 *       GF(251^2) -> GF(251^4): Y^2 - (X+1) = 0
 * 
 *  Complexity:
 *     9 multiplications + 4 doubles
 *        + 19 additions + 8 substractions + 4 reductions (on 32 bits)
 */
void gf251to4_mul(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    uint16_t m_ab0 = a[0]*b[0]; // <= p^2
    uint16_t m_ab1 = a[1]*b[1]; // <= p^2
    uint16_t m_ab2 = a[2]*b[2]; // <= p^2
    uint16_t m_ab3 = a[3]*b[3]; // <= p^2
    uint16_t a_a01 = a[0]+a[1]; // <= 2p
    uint16_t a_a23 = a[2]+a[3]; // <= 2p
    uint16_t a_a02 = a[0]+a[2]; // <= 2p
    uint16_t a_a13 = a[1]+a[3]; // <= 2p
    uint16_t a_b01 = b[0]+b[1]; // <= 2p
    uint16_t a_b23 = b[2]+b[3]; // <= 2p
    uint16_t a_b02 = b[0]+b[2]; // <= 2p
    uint16_t a_b13 = b[1]+b[3]; // <= 2p
    
    uint32_t cnst0 = 2*m_ab1 + m_ab0; // <= 3 p^2
    uint32_t cnst1 = a_a01*a_b01 + 2*PRIME*PRIME - m_ab1 - m_ab0; // <= 6 p^2
    uint32_t leading0 = 2*m_ab3 + m_ab2; // <= 3 p^2
    uint32_t leading1 = a_a23*a_b23 + 2*PRIME*PRIME - m_ab3 - m_ab2; // <= 6 p^2
    uint32_t cnst_sum = a_a02*a_b02; // <= 4 p^2
    uint32_t leading_sum = a_a13*a_b13; // <= 4 p^2
    uint32_t a_a01234 = a_a01 + a_a23; // <= 4 p
    uint32_t a_b01234 = a_b01 + a_b23; // <= 4 p

    uint32_t a_cnstleading0 = leading0 + cnst0; // <= 6 p^2
    uint32_t a_cnstleading1 = leading1 + cnst1; // <= 12 p^2

    uint32_t res0 = 2*leading1 + a_cnstleading0; // <= 18 p^2
    uint32_t res1 = a_cnstleading1 + leading0; // <= 15 p^2
    uint32_t res2 = 2*leading_sum + cnst_sum + 6*PRIME*PRIME - a_cnstleading0; // <= 18 p^2
    uint32_t res3 = a_a01234*a_b01234 + 20*PRIME*PRIME - leading_sum - cnst_sum - a_cnstleading1; // <= 36 p^2

    res[0] = (uint8_t)_gf251_reduce32(res0);
    res[1] = (uint8_t)_gf251_reduce32(res1);
    res[2] = (uint8_t)_gf251_reduce32(res2);
    res[3] = (uint8_t)_gf251_reduce32(res3);
}

#define SET_RES_REDUCE8(i,exp) {v=exp; res[i]=(uint8_t)(v-PRIME*(PRIME<=v));}
void gf251to4_add(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    uint16_t v;
    SET_RES_REDUCE8(0, a[0]+b[0]);
    SET_RES_REDUCE8(1, a[1]+b[1]);
    SET_RES_REDUCE8(2, a[2]+b[2]);
    SET_RES_REDUCE8(3, a[3]+b[3]);
}

void gf251to4_sub(uint8_t res[4], const uint8_t a[4], const uint8_t b[4]) {
    uint16_t v;
    SET_RES_REDUCE8(0, a[0]+PRIME-b[0]);
    SET_RES_REDUCE8(1, a[1]+PRIME-b[1]);
    SET_RES_REDUCE8(2, a[2]+PRIME-b[2]);
    SET_RES_REDUCE8(3, a[3]+PRIME-b[3]);
}

#define SET_RES_REDUCE16(i,exp) {v=exp; res[i]=(uint8_t)_gf251_reduce16(v);}
void gf251to4_mul_gf251(uint8_t res[4], uint8_t a, const uint8_t b[4]) {
    uint16_t v;
    SET_RES_REDUCE16(0, a * b[0]);
    SET_RES_REDUCE16(1, a * b[1]);
    SET_RES_REDUCE16(2, a * b[2]);
    SET_RES_REDUCE16(3, a * b[3]);
}

int gf251to4_eq(const uint8_t a[4], const uint8_t b[4]) {
    int res = 1;
    res &= (a[0] == b[0]);
    res &= (a[1] == b[1]);
    res &= (a[2] == b[2]);
    res &= (a[3] == b[3]);
    return res;
}
