#include "pack.h"

void pack(uint8_t *buf, const size_t buf_len,   const uint32_t *data, const size_t data_len){
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

void unpack(uint32_t *data, const size_t data_len,  const uint8_t *buf,const uint8_t *siged,const size_t buf_len){
    for(int i = 0; i < (int)data_len; i++){
        data[i] = ((uint32_t)(buf[i*2+0])) ^ ((uint32_t)(buf[i*2+1] << 8));
    }
    for(int i = 0; i < (int)data_len/8; i++){ 
        for(int j = 0; j < 8; j++){  
            data[i*8 + j] = ((((uint32_t)siged[i]) << (j+9)) & 0x10000) ^ data[i*8 + j];
        }
    }
}