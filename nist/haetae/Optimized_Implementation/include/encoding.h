#ifndef HAETAE_ENCODING_H
#define HAETAE_ENCODING_H

#include "params.h"
#include <stdint.h>

#define encode_h HAETAE_NAMESPACE(encode_h)
uint8_t *encode_h(uint8_t *buf, const int32_t *h);
#define decode_h HAETAE_NAMESPACE(decode_h)
void decode_h(int32_t *h, uint8_t *buf);
#define encode_hb_z1 HAETAE_NAMESPACE(encode_hb_z1)
uint8_t *encode_hb_z1(uint8_t *buf, const int32_t *hb_z1);
#define decode_hb_z1 HAETAE_NAMESPACE(decode_hb_z1)
void decode_hb_z1(int32_t *hb_z1, uint8_t *buf);

#endif // HAETAE_ENCODING_H
