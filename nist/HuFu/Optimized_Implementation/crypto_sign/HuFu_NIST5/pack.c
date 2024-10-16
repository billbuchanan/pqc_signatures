#include "pack.h"
#include "stdio.h"
#include"immintrin.h"
#include "util.h"
void pack_new(uint8_t *buf, const size_t buf_len,   const uint32_t *data, const size_t data_len){
        //buf_len = data_len*2+data_len/8;
        uint8_t *symbol = buf + data_len*2;
        for(int i = 0; i < (int)data_len; i++){
              buf[i*2+0] = (uint8_t) (data[i]);
              buf[i*2+1] = (uint8_t) (data[i] >> 8);
        }
        for(int i = 0; i < (int)data_len/8; i++){
            for(int j = 0; j < 8; j++){
                 symbol[i] = ((data[i*8+0]>>9)&0x80) ^ ((data[i*8+1]>>10)&0x40) ^ ((data[i*8+2]>>11)&0x20) ^ ((data[i*8+3]>>12)&0x10) ^ 
                             ((data[i*8+4]>>13)&0x08) ^ ((data[i*8+5]>>14)&0x04) ^ ((data[i*8+6]>>15)&0x02) ^ ((data[i*8+7]>>16)&0x01);
            }
        }

}

void unpack_new(uint32_t *data, const size_t data_len,  const uint8_t *buf,const uint8_t *siged,const size_t buf_len){
    for(int i = 0; i < (int)data_len; i++){
        data[i] = ((uint32_t)(buf[i*2+0])) ^ ((uint32_t)(buf[i*2+1] << 8));
    }
    for(int i = 0; i < (int)data_len/8; i++){ 
        for(int j = 0; j < 8; j++){  
            data[i*8 + j] = ((((uint32_t)siged[i]) << (j+9)) & 0x10000) ^ data[i*8 + j];
        }
    }
}
void mat_mul_unpack(int32_t * C, const uint8_t * A,const uint8_t *A1, const int32_t * B, int l1, int l2, int l3){
    int32_t *realA = malloc(l1 * l2 * sizeof(int32_t));
    unpack_new((uint32_t*)realA,l1*l2,A,A1,(l1*l2*17)>>3);
    mat_mul(C,realA,B,l1,l2,l3);
    free(realA);
}


// pack and unpack mat R, in which each element has two bits
void pack_mat_r(uint8_t *buf, const int32_t *data)
{
    for (size_t i = 0; i < PARAM_R_BYTES; i++)
    {
        buf[i] = ((data[0] & 0x03) << 6) | ((data[1] & 0x03) << 4) | ((data[2] & 0x03) << 2) | (data[3] & 0x03);
        data += 4;
    }
}


void unpack_mat_r(int32_t *data, const uint8_t *buf, int buf_len)
{
    for (size_t i = 0; i < (size_t)buf_len; i++)
    {
        data[0] = (buf[i] & 0xc0)>>6;
		if (data[0]==3){
			data[0] = -1;
		}
		data[1] = (buf[i] & 0x30)>>4;
		if (data[1]==3){
			data[1] = -1;
		}
        data[2] = (buf[i] & 0x0c)>>2;
		if (data[2]==3){
			data[2] = -1;
		}
        data[3] = (buf[i] & 0x03);
		if (data[3]==3){
			data[3] = -1;
		}
        data += 4;
    }
}