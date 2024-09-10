#include "gen_mat.h"
#include "aes/aes.h"
#include "sha3/fips202x4.h"
void gen_mat_aes256(uint8_t * data, int data_len, const uint8_t * m, int m_len){
        memset(data, 0, data_len);
        for (int i = 0; i < data_len / 16;i++){
                memcpy(data + 16 * i, &i, 4);
        }
        uint8_t aes_key_schedule[16*15];
        AES256_load_schedule(m, aes_key_schedule); 
        AES256_ECB_enc_sch(data, data_len, aes_key_schedule,data); 
        AES256_free_schedule(aes_key_schedule);
}

void gen_mat_shake256x4(uint8_t * data, int data_len, const uint8_t * m, int m_len){
        int line1 = data_len*1/4;
        int line2 = data_len*2/4;
        int line3 = data_len*3/4;
        shake256x4(data, data+line1, data+line2, data+line3, data_len/4, m, m_len);
}

#define GEN_MAT_AES256
void gen_mat(uint8_t * data, int data_len, const uint8_t * m, int m_len){        
        #if !defined(GEN_MAT_AES256)
        gen_mat_shake256x4(data, data_len, m, m_len);
        #else
        gen_mat_aes256(data, data_len, m, m_len); 
        #endif
}
