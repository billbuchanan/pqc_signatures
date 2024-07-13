#ifndef ROUND_H_
#define ROUND_H_

#include "ascon.h"
#include "constants.h"
#include "forceinline.h"
#include "printstate.h"
#include "word.h"

forceinline void ROUND(ascon_state_t* s, uint8_t C) {
  ascon_state_t t;
  /* round constant */
  s->x[2] ^= C;
  /* s-box layer */
  s->x[0] ^= s->x[4];
  s->x[4] ^= s->x[3];
  s->x[2] ^= s->x[1];
  t.x[0] = s->x[0] ^ (~s->x[1] & s->x[2]);
  t.x[2] = s->x[2] ^ (~s->x[3] & s->x[4]);
  t.x[4] = s->x[4] ^ (~s->x[0] & s->x[1]);
  t.x[1] = s->x[1] ^ (~s->x[2] & s->x[3]);
  t.x[3] = s->x[3] ^ (~s->x[4] & s->x[0]);
  t.x[1] ^= t.x[0];
  t.x[3] ^= t.x[2];
  t.x[0] ^= t.x[4];
  /* linear layer */
  s->x[2] = t.x[2] ^ ROR(t.x[2], 6 - 1);
  s->x[3] = t.x[3] ^ ROR(t.x[3], 17 - 10);
  s->x[4] = t.x[4] ^ ROR(t.x[4], 41 - 7);
  s->x[0] = t.x[0] ^ ROR(t.x[0], 28 - 19);
  s->x[1] = t.x[1] ^ ROR(t.x[1], 61 - 39);
  s->x[2] = t.x[2] ^ ROR(s->x[2], 1);
  s->x[3] = t.x[3] ^ ROR(s->x[3], 10);
  s->x[4] = t.x[4] ^ ROR(s->x[4], 7);
  s->x[0] = t.x[0] ^ ROR(s->x[0], 19);
  s->x[1] = t.x[1] ^ ROR(s->x[1], 39);
  s->x[2] = ~s->x[2];
  printstate(" round output", s);
}

forceinline void PROUNDS(ascon_state_t* s, int nr) {
  int i = START(nr);
  do {
    ROUND(s, (uint8_t)RC(i));
    i += INC;
  } while (i != END);
}

#endif /* ROUND_H_ */
