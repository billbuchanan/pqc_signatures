#include "p251p4.h"

#include "assertions.h"

// TODO: this table should be part of p251.h
static int sdith_p251_tables_initialized = 0;
static uint8_t sdith_p251_logtable[256];
static uint8_t sdith_p251_exptable[251];

#ifdef NO_PRECOMPUTE_TABLES
static int sdith_p251p2_tables_initialized = 0;
uint16_t sdith_p251p2_logtable[65536];
uint16_t sdith_p251p2_exptable[SDITH_ORDER_P251P2+1];
static int sdith_p251p4_tables_initialized = 0;
uint32_t sdith_p251p4_log1table[SDITH_ORDER_P251P2+1];     // for x=u+Y: table of log16(u) -> log32(u+Y)
uint16_t sdith_p251p4_exp1table[SDITH_PP_POWER_P251P4][2];  // table of p -> (log16(u),log16(v))  where gen32^p = u+vY
#else
extern uint16_t sdith_p251p2_logtable[65536];
extern uint16_t sdith_p251p2_exptable[SDITH_ORDER_P251P2+1];
extern uint32_t sdith_p251p4_log1table[SDITH_ORDER_P251P2+1]; 
extern uint16_t sdith_p251p4_exp1table[SDITH_PP_POWER_P251P4][2];
#endif

/** @brief naive multiplication in p251p2 */
uint16_t p251p2_mul_naive(uint16_t x, uint16_t y) {
  uint8_t* xx = (uint8_t*) &x;
  uint8_t* yy = (uint8_t*) &y;
  uint32_t z0 = p251_mul_naive(xx[0], yy[0]);
  uint32_t z1 = p251_mul_naive(xx[0], yy[1]) + p251_mul_naive(xx[1], yy[0]);
  uint32_t z2 = p251_mul_naive(xx[1], yy[1]);
  z0 += SDITH_IRRED_CST_P251P2 * z2;
  return p251_reduce_16(z0) | (p251_reduce_16(z1)<<8);
}

/** @brief naive addition in p251p4 */
uint16_t p251p2_add_naive(uint16_t x, uint16_t y) {
  uint8_t* xx = (uint8_t*) &x;
  uint8_t* yy = (uint8_t*) &y;
  uint16_t z0 = p251_reduce_16(xx[0] + yy[0]);
  uint16_t z1 = p251_reduce_16(xx[1] + yy[1]);
  return z0 | (z1<<8);
}

/** @brief multiplication in p251p4 (mod X^2-Y) */
uint32_t p251p4_mul_naive(uint32_t x, uint32_t y) {
  uint16_t* xx = (uint16_t*) &x;
  uint16_t* yy = (uint16_t*) &y;
  uint64_t z0 = p251p2_mul_naive(xx[0],yy[0]);
  uint64_t z1 = p251p2_add_naive(p251p2_mul_naive(xx[0],yy[1]), p251p2_mul_naive(xx[1],yy[0]));
  uint64_t z2 = p251p2_mul_naive(xx[1],yy[1]);
  z0 = p251p2_add_naive(z0, p251p2_mul_naive(SDITH_IRRED_CST_P251P4, z2));
  return z0 | (z1<<16);
}

/** @brief naive addition in p251p4 (mod X^2-Y) */
uint32_t p251p4_add_naive(uint32_t x, uint32_t y) {
  uint8_t* xx = (uint8_t*) &x;
  uint8_t* yy = (uint8_t*) &y;
  uint16_t z0 = p251_reduce_16(xx[0] + yy[0]);
  uint16_t z1 = p251_reduce_16(xx[1] + yy[1]);
  uint16_t z2 = p251_reduce_16(xx[2] + yy[2]);
  uint16_t z3 = p251_reduce_16(xx[3] + yy[3]);
  return z0 | (z1<<8) | (z2<<16) | (z3<<24);
}

/** @brief naive addition in p251p4 (mod X^2-Y) */
uint32_t p251p4_sub_naive(uint32_t x, uint32_t y) {
  uint8_t* xx = (uint8_t*) &x;
  uint8_t* yy = (uint8_t*) &y;
  uint16_t z0 = p251_reduce_16((uint16_t)xx[0] + 251 - yy[0]);
  uint16_t z1 = p251_reduce_16((uint16_t)xx[1] + 251 - yy[1]);
  uint16_t z2 = p251_reduce_16((uint16_t)xx[2] + 251 - yy[2]);
  uint16_t z3 = p251_reduce_16((uint16_t)xx[3] + 251 - yy[3]);
  return z0 | (z1<<8) | (z2<<16) | (z3<<24);
}

/** @brief naive (divide and conquer) pow in p251p4 */
uint32_t p251p4_pow_naive(uint32_t x, uint64_t n) {
  uint32_t y = x;
  uint32_t z = 1;
  // keep zy^n constant and decrease n->0
  while (n != 0) {
    if (n % 2 == 1) {
      z = p251p4_mul_naive(z, y);
    }
    y = p251p4_mul_naive(y, y);
    n /= 2;
  }
  return z;
}

/** @brief lookup-table based discrete log in p251p4 (non constant time)
 * Please initialize the p251p4 log tables once before using this function */
uint32_t p251p4_dlog(const uint32_t x) {
  const uint32_t lu = sdith_p251p2_logtable[x & 0xFFFF];
  const uint32_t lv = sdith_p251p2_logtable[x >> 16u];
  if (lv == SDITH_ORDER_P251P2) return SDITH_PP_POWER_P251P4 * lu;  // x is in p251p2
  const uint32_t ldiff = (lu == SDITH_ORDER_P251P2 ? SDITH_ORDER_P251P2 : (lu >= lv ? lu - lv : SDITH_ORDER_P251P2 + lu - lv));
  return ((uint64_t)(sdith_p251p4_log1table[ldiff]) + SDITH_PP_POWER_P251P4 * (uint64_t)(lv)) % SDITH_ORDER_P251P4;
}

/** @brief lookup-table based discrete exp in p251p4 (non constant time)
 * Please initialize the p251p4 log tables once before using this function */
uint32_t p251p4_dexp(const uint32_t x) {
  if (x == SDITH_ORDER_P251P4) return 0;
  // decompose x = K.lv + lu
  const uint32_t lv = x / SDITH_PP_POWER_P251P4;  // lv in [0,order-1]
  const uint32_t lu = x % SDITH_PP_POWER_P251P4;  // lu in [0,pppow-1]
  if (lu == 0) return sdith_p251p2_exptable[lv];
  uint16_t const* r = sdith_p251p4_exp1table[lu];
  const uint32_t lru = r[0] == SDITH_ORDER_P251P2 ? SDITH_ORDER_P251P2 : (r[0] + lv) % SDITH_ORDER_P251P2;
  const uint32_t lrv = r[1] == SDITH_ORDER_P251P2 ? SDITH_ORDER_P251P2 : (r[1] + lv) % SDITH_ORDER_P251P2;
  return ((uint32_t)(sdith_p251p2_exptable[lrv]) << 16) | (sdith_p251p2_exptable[lru]);
}

#define SDITH_ORDER_P251 250

void create_p251_log_tables() {
  if (sdith_p251_tables_initialized) return;
  sdith_p251_exptable[SDITH_ORDER_P251] = 0;
  sdith_p251_exptable[0] = 1;
  sdith_p251_exptable[1] = SDITH_GEN_P251;
  for (uint64_t i = 2; i < SDITH_ORDER_P251; ++i) {
    sdith_p251_exptable[i] = p251_mul_naive(sdith_p251_exptable[i - 1], SDITH_GEN_P251);
  }
  for (uint64_t i = 0; i <= SDITH_ORDER_P251; ++i) {
    sdith_p251_logtable[sdith_p251_exptable[i]] = i;
  }
  sdith_p251_tables_initialized = 1;
}

void create_p251p2_log_tables() {
#ifdef NO_PRECOMPUTE_TABLES
  if (sdith_p251p2_tables_initialized) return;
  create_p251_log_tables();
  sdith_p251p2_exptable[SDITH_ORDER_P251P2] = 0;
  sdith_p251p2_exptable[0] = 1;
  sdith_p251p2_exptable[1] = SDITH_GEN_P251P2;
  for (uint64_t i = 2; i < SDITH_PP_POWER_P251P2; ++i) {
    sdith_p251p2_exptable[i] = p251p2_mul_naive(sdith_p251p2_exptable[i - 1], SDITH_GEN_P251P2);
  }
  for (uint64_t i = 0; i < SDITH_PP_POWER_P251P2; ++i) {
    uint16_t l = sdith_p251p2_exptable[i];
    uint8_t l0 = sdith_p251_logtable[l & 0xFF];
    uint8_t l1 = sdith_p251_logtable[l >> 8];
    for (uint64_t j = 1; j < SDITH_ORDER_P251; ++j) {
      l0 = l0 == SDITH_ORDER_P251 ? SDITH_ORDER_P251 : l0 == SDITH_ORDER_P251-1 ? 0 : l0 + 1;
      l1 = l1 == SDITH_ORDER_P251 ? SDITH_ORDER_P251 : l1 == SDITH_ORDER_P251-1 ? 0 : l1 + 1;
      sdith_p251p2_exptable[i + SDITH_PP_POWER_P251P2 * j] =
          (uint16_t)(sdith_p251_exptable[l0]) | ((uint16_t)(sdith_p251_exptable[l1]) << 8);
    }
  }
  for (uint64_t i = 0; i <= SDITH_ORDER_P251P2; ++i) {
    sdith_p251p2_logtable[sdith_p251p2_exptable[i]] = i;
  }
  sdith_p251p2_tables_initialized = 1;
#endif
}

void create_p251p4_log_tables() {
#ifdef NO_PRECOMPUTE_TABLES
  if (sdith_p251p4_tables_initialized) return;
  create_p251p2_log_tables();
  uint32_t z = 1;
  uint16_t* zz = (uint16_t*)&z;
  // g^0 = 1 + 0y
  sdith_p251p4_exp1table[0][0] = 0x0000;
  sdith_p251p4_exp1table[0][1] = SDITH_ORDER_P251P2;
  for (uint64_t i = 1; i < SDITH_PP_POWER_P251P4; ++i) {
    // g^i = u + Y v.
    z = p251p4_mul_naive(z, SDITH_GEN_P251P4);
    uint16_t lu = sdith_p251p2_logtable[zz[0]];
    uint16_t lv = sdith_p251p2_logtable[zz[1]];
    sdith_p251p4_exp1table[i][0] = lu;
    sdith_p251p4_exp1table[i][1] = lv;
    //REQUIRE_DRAMATICALLY(lv!=SDITH_ORDER_P251P2, "bug!! v cannot be zero");
    uint16_t ldiff = (lu == SDITH_ORDER_P251P2) ? (uint16_t)SDITH_ORDER_P251P2 : lu >= lv ? (uint16_t)(lu - lv) : SDITH_ORDER_P251P2 + lu - lv;
    // g32^(i-K.lv) = (u/v) + y = g16^ldiff + Y
    sdith_p251p4_log1table[ldiff] = (lv == 0) ? i : SDITH_ORDER_P251P4 + i - SDITH_PP_POWER_P251P4 * lv;
    // REQUIRE_DRAMATICALLY(p251p4_pow_naive(SDITH_GEN_P251P4, sdith_p251p4_log1table[ldiff]) == 0x10000U +
    // sdith_p251p2_exptable[ldiff], "XXXX")
  }
  sdith_p251p4_tables_initialized = 1;
#endif

  // printf("uint16_t sdith_p251p4_log1table[63001] = {");
  // for (int i = 0; i < 63001; ++i) {
  //   printf("%u, ", sdith_p251p4_log1table[i]);
  // }
  // printf("};\n\n");

  // printf("uint16_t sdith_p251p4_exp1table[63002][2] = {");
  // for (int i = 0; i < 63002; ++i) {
  //   printf("{%u, %u}, ", sdith_p251p4_exp1table[i][0], sdith_p251p4_exp1table[i][1]);
  // }
  // printf("};\n\n");

  // abort();
}

uint32_t p251p4_dlog_pow_l32(uint32_t logx, uint32_t p) {
  if (logx == SDITH_ORDER_P251P4) return SDITH_ORDER_P251P4;
  return ((uint64_t)p * (uint64_t)logx) % SDITH_ORDER_P251P4;
}

uint32_t p251p4_pow(uint32_t x, uint32_t p) {
  return p251p4_dexp(p251p4_dlog_pow_l32(p251p4_dlog(x), p));
}

uint32_t p251p4_dlog_mul_l32_l32(uint32_t logx, uint32_t logy) {
  if (logx == SDITH_ORDER_P251P4 || logy == SDITH_ORDER_P251P4) return SDITH_ORDER_P251P4;
  uint64_t l = (uint64_t)logx + (uint64_t)logy;
  if (l >= SDITH_ORDER_P251P4) l -= SDITH_ORDER_P251P4;
  return l;
}

uint32_t p251p4_mul_table(uint32_t x, uint32_t y) {
    return p251p4_dexp(p251p4_dlog_mul_l32_l32(p251p4_dlog(x), p251p4_dlog(y)));
}

uint32_t (*p251p4_mul_ct)(uint32_t x, uint32_t y) = 0;
uint32_t (*p251p4_mul)(uint32_t x, uint32_t y) = 0;

EXPORT void p251p4_mul_init() {
  create_p251p4_log_tables();
  p251p4_mul_ct = p251p4_mul_naive;
  p251p4_mul = p251p4_mul_table;
}
