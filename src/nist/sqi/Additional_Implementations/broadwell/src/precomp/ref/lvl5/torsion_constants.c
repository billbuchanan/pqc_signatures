#include <stddef.h>
#include <stdint.h>
#include <torsion_constants.h>
#if 0
#elif 8*DIGIT_LEN == 16
const uint64_t TORSION_PLUS_EVEN_POWER = 0x91;
const uint64_t TORSION_ODD_PRIMES[34] = {0x3, 0xd, 0x29, 0x43, 0x67, 0x1cd, 0x33b, 0x5, 0x7, 0x11, 0x25, 0x35, 0x49, 0x7f, 0x97, 0x283, 0x2dd, 0x2e3, 0x3f1, 0x9eb, 0x1039, 0x13c3, 0x1bd7, 0x2965, 0x3517, 0x3a4b, 0x3cb5, 0x3e77, 0x4897, 0x5abb, 0xbc3b, 0xf70f, 0xfff1, 0x4db19};
const uint64_t TORSION_ODD_POWERS[34] = {0x48, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
const uint64_t TORSION_PLUS_ODD_PRIMES[7] = {0x3, 0xd, 0x29, 0x43, 0x67, 0x1cd, 0x33b};
const size_t TORSION_PLUS_ODD_POWERS[7] = {0x48, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6};
const uint64_t TORSION_MINUS_ODD_PRIMES[27] = {0x5, 0x7, 0x11, 0x25, 0x35, 0x49, 0x7f, 0x97, 0x283, 0x2dd, 0x2e3, 0x3f1, 0x9eb, 0x1039, 0x13c3, 0x1bd7, 0x2965, 0x3517, 0x3a4b, 0x3cb5, 0x3e77, 0x4897, 0x5abb, 0xbc3b, 0xf70f, 0xfff1, 0x4db19};
const size_t TORSION_MINUS_ODD_POWERS[27] = {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
const size_t DEGREE_COMMITMENT_POWERS[34] = {0x0, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
const ibz_t CHARACTERISTIC = {{._mp_alloc = 0, ._mp_size = 32, ._mp_d = (mp_limb_t[]) {0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xa6e1,0x68ad,0x994c,0x74ce,0x7819,0xa29a,0xfaf0,0xea65,0x4a0d,0xc590,0xfe3a,0x7d01,0x2650,0xbe63,0x2bd,0xe792,0x6936,0xb003,0x8c15,0x9bc6,0xa886,0x5946,0x25}}};
const ibz_t TORSION_ODD = {{._mp_alloc = 0, ._mp_size = 41, ._mp_d = (mp_limb_t[]) {0x6443,0x4509,0x1585,0x113b,0x6c1d,0xac5,0x133e,0xbacd,0xbe1d,0xc40e,0x6dbd,0x1dab,0x7f88,0x84f7,0x9a8a,0xa77f,0xc9a3,0x9952,0x1850,0xa5c9,0x61d3,0xead,0x4f3e,0x684b,0x56d7,0x8f3d,0x6e0b,0x5c5b,0xed75,0x2a69,0xb601,0x6fb3,0x773f,0x44fa,0xc95c,0x5720,0x1ac,0x6b82,0xf6d9,0xf93,0x78}}};
const ibz_t TORSION_ODD_PRIMEPOWERS[34] = {{{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0xf6a1,0xc929,0x9735,0x1e67,0x6c11,0x60e7,0x56bc,0x4}}}, {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0xa6b9,0x49}}}, {{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0xd6b1,0x1b20,0x1}}}, {{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x5f59,0xfbd,0x15}}}, {{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x1f51,0x310,0x116}}}, {{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0x6739,0x2acb,0x19d4,0x22}}}, {{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0xa389,0x2c91,0x8ff6,0x470}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x5}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x7}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x11}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x25}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x35}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x49}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x7f}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x97}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x283}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2dd}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2e3}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3f1}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x9eb}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1039}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x13c3}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1bd7}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2965}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3517}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3a4b}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3cb5}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3e77}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x4897}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x5abb}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xbc3b}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xf70f}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xfff1}}}, {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0xdb19,0x4}}}};
const ibz_t TORSION_ODD_PLUS = {{._mp_alloc = 0, ._mp_size = 23, ._mp_d = (mp_limb_t[]) {0xd371,0x3456,0x4ca6,0xba67,0x3c0c,0x514d,0xfd78,0xf532,0x2506,0x62c8,0xff1d,0x3e80,0x9328,0xdf31,0x15e,0x73c9,0xb49b,0xd801,0x460a,0x4de3,0x5443,0xaca3,0x12}}};
const ibz_t TORSION_ODD_MINUS = {{._mp_alloc = 0, ._mp_size = 19, ._mp_d = (mp_limb_t[]) {0xb0f3,0xa6d1,0x9048,0x1def,0x2075,0x4e99,0xa926,0xc786,0x4dda,0xdcb1,0xa4b,0x6123,0xe306,0x599e,0xfd2a,0x844f,0xae9c,0x6dde,0x6}}};
const ibz_t TORSION_PLUS_2POWER = {{._mp_alloc = 0, ._mp_size = 10, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x2}}};
const ibz_t TORSION_PLUS_3POWER = {{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0xf6a1,0xc929,0x9735,0x1e67,0x6c11,0x60e7,0x56bc,0x4}}};
const ibz_t TORSION_PLUS_23POWER = {{._mp_alloc = 0, ._mp_size = 17, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xed42,0x9253,0x2e6b,0x3ccf,0xd822,0xc1ce,0xad78,0x8}}};
const ibz_t DEGREE_COMMITMENT = {{._mp_alloc = 0, ._mp_size = 34, ._mp_d = (mp_limb_t[]) {0x8463,0x2824,0xc02f,0xad88,0x1daa,0x1fe6,0x44e3,0x41c4,0xd324,0x8315,0xd382,0x1f79,0xf852,0x980a,0x9bf7,0x3bf8,0xe38f,0x4576,0xb36,0x7082,0x9fe4,0x3c,0x63db,0x611f,0x3261,0x345,0x3575,0xfefa,0xf00e,0x8537,0x712c,0x9f1c,0xabde,0x1b}}};
const ibz_t DEGREE_COMMITMENT_PLUS = {{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0x3ad1,0x4eb4,0x7a3,0x1f00,0x1d6a,0x7d31,0x865,0xc7bc,0x9962,0xf08a,0x13f9,0xe003,0x6d0e,0xc440,0x4dd4,0x4}}};
const ibz_t DEGREE_COMMITMENT_MINUS = {{._mp_alloc = 0, ._mp_size = 19, ._mp_d = (mp_limb_t[]) {0xb0f3,0xa6d1,0x9048,0x1def,0x2075,0x4e99,0xa926,0xc786,0x4dda,0xdcb1,0xa4b,0x6123,0xe306,0x599e,0xfd2a,0x844f,0xae9c,0x6dde,0x6}}};
const ibz_t DEGREE_CHALLENGE = {{._mp_alloc = 0, ._mp_size = 17, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xed42,0x9253,0x2e6b,0x3ccf,0xd822,0xc1ce,0xad78,0x8}}};
#elif 8*DIGIT_LEN == 32
const uint64_t TORSION_PLUS_EVEN_POWER = 0x91;
const uint64_t TORSION_ODD_PRIMES[34] = {0x3, 0xd, 0x29, 0x43, 0x67, 0x1cd, 0x33b, 0x5, 0x7, 0x11, 0x25, 0x35, 0x49, 0x7f, 0x97, 0x283, 0x2dd, 0x2e3, 0x3f1, 0x9eb, 0x1039, 0x13c3, 0x1bd7, 0x2965, 0x3517, 0x3a4b, 0x3cb5, 0x3e77, 0x4897, 0x5abb, 0xbc3b, 0xf70f, 0xfff1, 0x4db19};
const uint64_t TORSION_ODD_POWERS[34] = {0x48, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
const uint64_t TORSION_PLUS_ODD_PRIMES[7] = {0x3, 0xd, 0x29, 0x43, 0x67, 0x1cd, 0x33b};
const size_t TORSION_PLUS_ODD_POWERS[7] = {0x48, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6};
const uint64_t TORSION_MINUS_ODD_PRIMES[27] = {0x5, 0x7, 0x11, 0x25, 0x35, 0x49, 0x7f, 0x97, 0x283, 0x2dd, 0x2e3, 0x3f1, 0x9eb, 0x1039, 0x13c3, 0x1bd7, 0x2965, 0x3517, 0x3a4b, 0x3cb5, 0x3e77, 0x4897, 0x5abb, 0xbc3b, 0xf70f, 0xfff1, 0x4db19};
const size_t TORSION_MINUS_ODD_POWERS[27] = {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
const size_t DEGREE_COMMITMENT_POWERS[34] = {0x0, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
const ibz_t CHARACTERISTIC = {{._mp_alloc = 0, ._mp_size = 16, ._mp_d = (mp_limb_t[]) {0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xa6e1ffff,0x994c68ad,0x781974ce,0xfaf0a29a,0x4a0dea65,0xfe3ac590,0x26507d01,0x2bdbe63,0x6936e792,0x8c15b003,0xa8869bc6,0x255946}}};
const ibz_t TORSION_ODD = {{._mp_alloc = 0, ._mp_size = 21, ._mp_d = (mp_limb_t[]) {0x45096443,0x113b1585,0xac56c1d,0xbacd133e,0xc40ebe1d,0x1dab6dbd,0x84f77f88,0xa77f9a8a,0x9952c9a3,0xa5c91850,0xead61d3,0x684b4f3e,0x8f3d56d7,0x5c5b6e0b,0x2a69ed75,0x6fb3b601,0x44fa773f,0x5720c95c,0x6b8201ac,0xf93f6d9,0x78}}};
const ibz_t TORSION_ODD_PRIMEPOWERS[34] = {{{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0xc929f6a1,0x1e679735,0x60e76c11,0x456bc}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x49a6b9}}}, {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x1b20d6b1,0x1}}}, {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0xfbd5f59,0x15}}}, {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x3101f51,0x116}}}, {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x2acb6739,0x2219d4}}}, {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x2c91a389,0x4708ff6}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x5}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x7}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x11}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x25}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x35}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x49}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x7f}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x97}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x283}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2dd}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2e3}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3f1}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x9eb}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1039}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x13c3}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1bd7}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2965}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3517}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3a4b}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3cb5}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3e77}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x4897}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x5abb}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xbc3b}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xf70f}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xfff1}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x4db19}}}};
const ibz_t TORSION_ODD_PLUS = {{._mp_alloc = 0, ._mp_size = 12, ._mp_d = (mp_limb_t[]) {0x3456d371,0xba674ca6,0x514d3c0c,0xf532fd78,0x62c82506,0x3e80ff1d,0xdf319328,0x73c9015e,0xd801b49b,0x4de3460a,0xaca35443,0x12}}};
const ibz_t TORSION_ODD_MINUS = {{._mp_alloc = 0, ._mp_size = 10, ._mp_d = (mp_limb_t[]) {0xa6d1b0f3,0x1def9048,0x4e992075,0xc786a926,0xdcb14dda,0x61230a4b,0x599ee306,0x844ffd2a,0x6ddeae9c,0x6}}};
const ibz_t TORSION_PLUS_2POWER = {{._mp_alloc = 0, ._mp_size = 5, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0x20000}}};
const ibz_t TORSION_PLUS_3POWER = {{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0xc929f6a1,0x1e679735,0x60e76c11,0x456bc}}};
const ibz_t TORSION_PLUS_23POWER = {{._mp_alloc = 0, ._mp_size = 9, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0xed420000,0x2e6b9253,0xd8223ccf,0xad78c1ce,0x8}}};
const ibz_t DEGREE_COMMITMENT = {{._mp_alloc = 0, ._mp_size = 17, ._mp_d = (mp_limb_t[]) {0x28248463,0xad88c02f,0x1fe61daa,0x41c444e3,0x8315d324,0x1f79d382,0x980af852,0x3bf89bf7,0x4576e38f,0x70820b36,0x3c9fe4,0x611f63db,0x3453261,0xfefa3575,0x8537f00e,0x9f1c712c,0x1babde}}};
const ibz_t DEGREE_COMMITMENT_PLUS = {{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0x4eb43ad1,0x1f0007a3,0x7d311d6a,0xc7bc0865,0xf08a9962,0xe00313f9,0xc4406d0e,0x44dd4}}};
const ibz_t DEGREE_COMMITMENT_MINUS = {{._mp_alloc = 0, ._mp_size = 10, ._mp_d = (mp_limb_t[]) {0xa6d1b0f3,0x1def9048,0x4e992075,0xc786a926,0xdcb14dda,0x61230a4b,0x599ee306,0x844ffd2a,0x6ddeae9c,0x6}}};
const ibz_t DEGREE_CHALLENGE = {{._mp_alloc = 0, ._mp_size = 9, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x0,0x0,0xed420000,0x2e6b9253,0xd8223ccf,0xad78c1ce,0x8}}};
#elif 8*DIGIT_LEN == 64
const uint64_t TORSION_PLUS_EVEN_POWER = 0x91;
const uint64_t TORSION_ODD_PRIMES[34] = {0x3, 0xd, 0x29, 0x43, 0x67, 0x1cd, 0x33b, 0x5, 0x7, 0x11, 0x25, 0x35, 0x49, 0x7f, 0x97, 0x283, 0x2dd, 0x2e3, 0x3f1, 0x9eb, 0x1039, 0x13c3, 0x1bd7, 0x2965, 0x3517, 0x3a4b, 0x3cb5, 0x3e77, 0x4897, 0x5abb, 0xbc3b, 0xf70f, 0xfff1, 0x4db19};
const uint64_t TORSION_ODD_POWERS[34] = {0x48, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
const uint64_t TORSION_PLUS_ODD_PRIMES[7] = {0x3, 0xd, 0x29, 0x43, 0x67, 0x1cd, 0x33b};
const size_t TORSION_PLUS_ODD_POWERS[7] = {0x48, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6};
const uint64_t TORSION_MINUS_ODD_PRIMES[27] = {0x5, 0x7, 0x11, 0x25, 0x35, 0x49, 0x7f, 0x97, 0x283, 0x2dd, 0x2e3, 0x3f1, 0x9eb, 0x1039, 0x13c3, 0x1bd7, 0x2965, 0x3517, 0x3a4b, 0x3cb5, 0x3e77, 0x4897, 0x5abb, 0xbc3b, 0xf70f, 0xfff1, 0x4db19};
const size_t TORSION_MINUS_ODD_POWERS[27] = {0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
const size_t DEGREE_COMMITMENT_POWERS[34] = {0x0, 0x6, 0x6, 0x6, 0x6, 0x6, 0x6, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1};
const ibz_t CHARACTERISTIC = {{._mp_alloc = 0, ._mp_size = 8, ._mp_d = (mp_limb_t[]) {0xffffffffffffffff,0xffffffffffffffff,0x994c68ada6e1ffff,0xfaf0a29a781974ce,0xfe3ac5904a0dea65,0x2bdbe6326507d01,0x8c15b0036936e792,0x255946a8869bc6}}};
const ibz_t TORSION_ODD = {{._mp_alloc = 0, ._mp_size = 11, ._mp_d = (mp_limb_t[]) {0x113b158545096443,0xbacd133e0ac56c1d,0x1dab6dbdc40ebe1d,0xa77f9a8a84f77f88,0xa5c918509952c9a3,0x684b4f3e0ead61d3,0x5c5b6e0b8f3d56d7,0x6fb3b6012a69ed75,0x5720c95c44fa773f,0xf93f6d96b8201ac,0x78}}};
const ibz_t TORSION_ODD_PRIMEPOWERS[34] = {{{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x1e679735c929f6a1,0x456bc60e76c11}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x49a6b9}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x11b20d6b1}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x150fbd5f59}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x11603101f51}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2219d42acb6739}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x4708ff62c91a389}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x5}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x7}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x11}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x25}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x35}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x49}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x7f}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x97}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x283}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2dd}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2e3}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3f1}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x9eb}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1039}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x13c3}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x1bd7}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x2965}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3517}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3a4b}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3cb5}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x3e77}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x4897}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x5abb}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xbc3b}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xf70f}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0xfff1}}}, {{._mp_alloc = 0, ._mp_size = 1, ._mp_d = (mp_limb_t[]) {0x4db19}}}};
const ibz_t TORSION_ODD_PLUS = {{._mp_alloc = 0, ._mp_size = 6, ._mp_d = (mp_limb_t[]) {0xba674ca63456d371,0xf532fd78514d3c0c,0x3e80ff1d62c82506,0x73c9015edf319328,0x4de3460ad801b49b,0x12aca35443}}};
const ibz_t TORSION_ODD_MINUS = {{._mp_alloc = 0, ._mp_size = 5, ._mp_d = (mp_limb_t[]) {0x1def9048a6d1b0f3,0xc786a9264e992075,0x61230a4bdcb14dda,0x844ffd2a599ee306,0x66ddeae9c}}};
const ibz_t TORSION_PLUS_2POWER = {{._mp_alloc = 0, ._mp_size = 3, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x20000}}};
const ibz_t TORSION_PLUS_3POWER = {{._mp_alloc = 0, ._mp_size = 2, ._mp_d = (mp_limb_t[]) {0x1e679735c929f6a1,0x456bc60e76c11}}};
const ibz_t TORSION_PLUS_23POWER = {{._mp_alloc = 0, ._mp_size = 5, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x2e6b9253ed420000,0xad78c1ced8223ccf,0x8}}};
const ibz_t DEGREE_COMMITMENT = {{._mp_alloc = 0, ._mp_size = 9, ._mp_d = (mp_limb_t[]) {0xad88c02f28248463,0x41c444e31fe61daa,0x1f79d3828315d324,0x3bf89bf7980af852,0x70820b364576e38f,0x611f63db003c9fe4,0xfefa357503453261,0x9f1c712c8537f00e,0x1babde}}};
const ibz_t DEGREE_COMMITMENT_PLUS = {{._mp_alloc = 0, ._mp_size = 4, ._mp_d = (mp_limb_t[]) {0x1f0007a34eb43ad1,0xc7bc08657d311d6a,0xe00313f9f08a9962,0x44dd4c4406d0e}}};
const ibz_t DEGREE_COMMITMENT_MINUS = {{._mp_alloc = 0, ._mp_size = 5, ._mp_d = (mp_limb_t[]) {0x1def9048a6d1b0f3,0xc786a9264e992075,0x61230a4bdcb14dda,0x844ffd2a599ee306,0x66ddeae9c}}};
const ibz_t DEGREE_CHALLENGE = {{._mp_alloc = 0, ._mp_size = 5, ._mp_d = (mp_limb_t[]) {0x0,0x0,0x2e6b9253ed420000,0xad78c1ced8223ccf,0x8}}};
#endif