#include <stdint.h>
#include <string.h>
#include <math.h>

#include "params.h"

void pack(uint8_t *buf, const size_t buf_len, const uint32_t *data, const size_t data_len);

void unpack(uint32_t *data, const size_t data_len, const uint8_t *buf, const uint8_t *siged, const size_t buf_len);