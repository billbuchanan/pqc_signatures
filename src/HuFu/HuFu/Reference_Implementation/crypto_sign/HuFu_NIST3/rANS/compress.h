#include <stdint.h>

#include "../params.h"

// void test_compress(const int32_t * sig){
// 	clock_t t0, t1;
	
// 	uint8_t compressed_sig[PARAM_COMPRESSED_SIG_BYTES]={};
// 	int32_t decoded_sig[PARAM_M+PARAM_N]={};
	
// 	t0 = clock();
// 	compress_sig(compressed_sig,sig);
// 	decompress_sig(decoded_sig, compressed_sig);
// 	t1 = clock();
	
// 	printf("\nCompress time: %.2f ms\n\n", (t1 - t0) / 1000.0);
// 	printf("\nRaw Length: %lu Btyes, Compressed Length: %d Bytes\n\n", (PARAM_M + PARAM_N) * sizeof(int32_t),PARAM_COMPRESSED_SIG_BYTES);

//     for(int i=0;i<PARAM_M+PARAM_N;i++){
// 		if(decoded_sig[i]!=sig[i]){
// 			printf("DECODE ERROR!\n");
// 		}
// 	}
// }

// compress sig to buf
// buf should be AT LEAST PARAM_SIG_ENCODE_MAX_BYTES
// return # of used bytes in buf, if expection occurred, return 0
// NOTE: the buf is used in the REVERSE order
// buf: | unused bytes  | rANS(hbs) | lb_(M+N) | ... | lb_0 |
//      |               |<---------- # of used bytes ------>|
uint16_t compress_sig(uint8_t buf[PARAM_SIG_ENCODE_MAX_BYTES], const int32_t sig[PARAM_SIG_SIZE]);

// decompress sig from buf
// buf should point the first used bytes
// return 1 if decompress is successful. If exception occurred, return 0.
// buf: | rANS(hbs) | lb_(M+N) | ... | lb_0 |
//      |<------------- buf_len ----------->|
int decompress_sig(int32_t sig[PARAM_SIG_SIZE], const uint8_t *buf, const uint16_t buf_len);