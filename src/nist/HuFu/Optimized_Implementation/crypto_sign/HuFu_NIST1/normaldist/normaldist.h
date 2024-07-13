#include <stddef.h>

// void test_normal(){
// 	clock_t t0, t1;
// 	const int ITER = 1024;

// 	double vec[PARAM_N*ITER],*ptr=vec;

// 	double mean = 0., stddev = 0.;
// 	t0 = clock();
// 	for(int j=0; j<ITER; j++) {
// 		normaldist(ptr);
// 		ptr+=PARAM_N;
// 	}
// 	t1 = clock();

// 	for(int i=0; i<PARAM_N*ITER; i++) {
// 		mean   += vec[i];
// 		stddev += vec[i] * vec[i];
// 	}
// 	mean /= (PARAM_N * ITER);
// 	stddev = sqrt(stddev / (PARAM_N * ITER));
// 	printf("mean   = %f (expecting 0)\nstddev = %f (expecting 1)\n",
// 		mean, stddev);
// 	printf("Sample for %d times: %.2f ms\n",ITER, (t1 - t0) / 1000.0);
// }
void normaldist(double *vec, size_t n, uint8_t seed[32]);
