#include <stdint.h>
#include <stddef.h>
#include "bitstream.h"
#include "../fq_arithmetic/vf3.h"
#include "../params.h"

uint32_t tab_enc_val[27] = {
  0b10110110, 0b00010110, 0b1110110, 0b01010110, 0b110010, 0b11110, 0b10010110, 0b010010, 0b01110, 0b00110110, 0b101010, 0b100010, 0b100110, 0b1100, 0b111, 0b111010, 0b1000, 0b101, 0b11010110, 0b001010, 0b000010, 0b000110, 0b0100, 0b011, 0b011010, 0b0000, 0b001
};

uint8_t tab_enc_len[27] = {
  8, 8, 7, 8, 6, 5, 8, 6, 5, 8, 6, 6, 6, 4, 3, 6, 4, 3, 8, 6, 6, 6, 4, 3, 6, 4, 3
};

int compress(bitstream_t *stream, vf3_e *vec)
{
  int ret = 0;

  for (int i = 0; i+2 < vec->size; i += 3){
    ret |= bs_write(stream, tab_enc_val[vf3_get_element(i + 0, vec) * 1 + vf3_get_element(i + 1, vec) * 3 + vf3_get_element(i + 2, vec) * 9], tab_enc_len[vf3_get_element(i + 0, vec) * 1 + vf3_get_element(i + 1, vec) * 3 + vf3_get_element(i + 2, vec) * 9]);
}
  for (int i = 0; i < vec->size % 3; i++)
    ret |= bs_write(stream, vf3_get_element(vec->size - (vec->size % 3) + i, vec), 2);

  return ret;
}

int decompress(bitstream_t *stream, vf3_e *vec)
{
  uint32_t pos = 0;

  for (int i = 0; i+2 < vec->size; i += 3)
  {
    uint32_t val;

    val = bs_read(stream, 3);
    switch (val) {
      case 0b111:
        vf3_set_coeff(pos++, vec, 2);
        vf3_set_coeff(pos++, vec, 1);
        vf3_set_coeff(pos++, vec, 1);
      break;
      case 0b011:
        vf3_set_coeff(pos++, vec, 2);
        vf3_set_coeff(pos++, vec, 1);
        vf3_set_coeff(pos++, vec, 2);
      break;
      case 0b101:
        vf3_set_coeff(pos++, vec, 2);
        vf3_set_coeff(pos++, vec, 2);
        vf3_set_coeff(pos++, vec, 1);
      break;
      case 0b001:
        vf3_set_coeff(pos++, vec, 2);
        vf3_set_coeff(pos++, vec, 2);
        vf3_set_coeff(pos++, vec, 2);
      break;
      case 0b110:
        val = bs_read(stream, 2);
        switch (val) {
          case 0b11:
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 0);
          break;
          case 0b01:
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 0);
          break;
          case 0b10:
            val = bs_read(stream, 2);
            switch (val) {
              case 0b11:
                vf3_set_coeff(pos++, vec, 2);
                vf3_set_coeff(pos++, vec, 0);
                vf3_set_coeff(pos++, vec, 0);
              break;
              case 0b01:
                val = bs_read(stream, 1);
                switch (val) {
                  case 0b1:
                    vf3_set_coeff(pos++, vec, 0);
                    vf3_set_coeff(pos++, vec, 0);
                    vf3_set_coeff(pos++, vec, 0);
                  break;
                  case 0b0:
                    vf3_set_coeff(pos++, vec, 0);
                    vf3_set_coeff(pos++, vec, 0);
                    vf3_set_coeff(pos++, vec, 1);
                  break;
                }
              break;
              case 0b10:
                val = bs_read(stream, 1);
                switch (val) {
                  case 0b1:
                    vf3_set_coeff(pos++, vec, 0);
                    vf3_set_coeff(pos++, vec, 0);
                    vf3_set_coeff(pos++, vec, 2);
                  break;
                  case 0b0:
                    vf3_set_coeff(pos++, vec, 0);
                    vf3_set_coeff(pos++, vec, 1);
                    vf3_set_coeff(pos++, vec, 0);
                  break;
                }
              break;
              case 0b00:
                val = bs_read(stream, 1);
                switch (val) {
                  case 0b1:
                    vf3_set_coeff(pos++, vec, 0);
                    vf3_set_coeff(pos++, vec, 2);
                    vf3_set_coeff(pos++, vec, 0);
                  break;
                  case 0b0:
                    vf3_set_coeff(pos++, vec, 1);
                    vf3_set_coeff(pos++, vec, 0);
                    vf3_set_coeff(pos++, vec, 0);
                  break;
                }
              break;
            }
          break;
          case 0b00:
            val = bs_read(stream, 1);
            switch (val) {
              case 0b1:
                vf3_set_coeff(pos++, vec, 0);
                vf3_set_coeff(pos++, vec, 1);
                vf3_set_coeff(pos++, vec, 1);
              break;
              case 0b0:
                vf3_set_coeff(pos++, vec, 0);
                vf3_set_coeff(pos++, vec, 1);
                vf3_set_coeff(pos++, vec, 2);
              break;
            }
          break;
        }
      break;
      case 0b010:
        val = bs_read(stream, 3);
        switch (val) {
          case 0b111:
            vf3_set_coeff(pos++, vec, 0);
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 1);
          break;
          case 0b011:
            vf3_set_coeff(pos++, vec, 0);
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 2);
          break;
          case 0b101:
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 0);
            vf3_set_coeff(pos++, vec, 1);
          break;
          case 0b001:
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 0);
            vf3_set_coeff(pos++, vec, 2);
          break;
          case 0b110:
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 0);
          break;
          case 0b010:
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 0);
          break;
          case 0b100:
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 0);
            vf3_set_coeff(pos++, vec, 1);
          break;
          case 0b000:
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 0);
            vf3_set_coeff(pos++, vec, 2);
          break;
        }
      break;
      case 0b100:
        val = bs_read(stream, 1);
        switch (val) {
          case 0b1:
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 1);
          break;
          case 0b0:
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 2);
          break;
        }
      break;
      case 0b000:
        val = bs_read(stream, 1);
        switch (val) {
          case 0b1:
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 1);
          break;
          case 0b0:
            vf3_set_coeff(pos++, vec, 1);
            vf3_set_coeff(pos++, vec, 2);
            vf3_set_coeff(pos++, vec, 2);
          break;
        }
      break;
    }
  }

  if (vec->size % 3 == 1)
  {
    vf3_set_coeff(vec->size-1, vec, bs_read(stream, 2));
  }

  if (vec->size % 3 == 2)
  {
    vf3_set_coeff(vec->size-2, vec, bs_read(stream, 2));
    vf3_set_coeff(vec->size-1, vec, bs_read(stream, 2));
  }


  return 0;
}

