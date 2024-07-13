#include "gen_mat.h"
#include "aes/aes.h"
#include "sha3/fips202.h"


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

void gen_mat_shake256(uint8_t * data, int data_len, const uint8_t * m, int m_len){
	uint8_t *t;
        t = malloc(m_len+1);
        memcpy(t+1, m, m_len);
  
        for(int i=0; i<4; i++){
            t[0] = i;
            shake256(data, data_len*1/4, t, m_len+1);
            data = data+(data_len/4);
        }
}

#define GEN_MAT_AES256
void gen_mat(uint8_t * data, int data_len, const uint8_t * m, int m_len){        
        #if !defined(GEN_MAT_AES256)
        gen_mat_shake256(data, data_len, m, m_len);
        #else
        gen_mat_aes256(data, data_len, m, m_len); 
        #endif
}