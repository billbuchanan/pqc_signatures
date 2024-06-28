#include <gmp.h>
#include <time.h>
#include <sys/syscall.h>
#include <random>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include "sieve_constants.h"

using namespace std;

#define LOG3 1.58496250072116
#define LOG5 2.32192809488736
#define LOG7 2.80735492205760

uint trial_division(mpz_t n, int smoothness) {
  int num_primes[8] = {171, 308, 563, 1027, 1899, 3511, 6541, 12250};
  int np = num_primes[smoothness - 10];
  
  // Remove powers of 2
  mpz_tdiv_q_2exp(n, n, mpz_scan1(n, 0));
  //Remove smooth factors via trial division
  for (int i = 0; i < np; i++) {
    while (mpz_divisible_ui_p(n, primes[i])) {
      mpz_divexact_ui(n, n, primes[i]);
    }
  }
  // return size of non-smooth factor of n
  int bits = mpz_sizeinbase(n, 2);
  return bits == 1 ? 0 : bits;
}

void get_prime(mpz_t p, mpz_t L, mpz_t large, int degree, mpz_t one){ 
  //compute prime from output of XGCD:
  //p = 2(|L|*large)^degree - 1

  mpz_t tmp;
  mpz_init(tmp);

  mpz_mul(p, L, large);  // p = L * large = x
  mpz_abs(p, p);         // in case L is negative
  mpz_set(tmp, p);
  // compute p = x^degree 
  for (int i = 1; i < degree; i++){ 
    mpz_mul(p, p, tmp);
  }           
  mpz_add(p, p, p);      // p = 2x^degree
  mpz_sub(p, p, one);    // p = 2x^degree - 1 
}


bool check_p(mpz_t p, int smoothness, mpz_t one){
  //on input of p,
  //compute p^2-1 and check for smoothness

  int bits, p_bits_r, threshold_r, bits2;
  mpz_t tmp, tmp2;
  mpz_init(tmp); mpz_init(tmp2);
  
  p_bits_r = mpz_sizeinbase(p, 2); //get bitlength of p
  threshold_r = 1.25 * p_bits_r; //we need an odd smooth factor of size ~ p^(5/4)
  
  mpz_set(tmp, p);
  mpz_mul(tmp, tmp, tmp); //tmp = p^2
  mpz_sub(tmp, tmp, one); //tmp = p^2-1
  mpz_set(tmp2, tmp); //copy for checking power of 2

  // check for smooth factor of p^2-1
  bits = trial_division(tmp, smoothness); //returns size of non-smooth factor!
  
  //check for power of 2
  mpz_tdiv_q_2exp(tmp2, tmp2, mpz_scan1(tmp2, 0)); //remove powers of 2
  bits2 = mpz_sizeinbase(tmp2, 2);  //remaining bits

  return (bits2 - bits >= threshold_r); //check that odd part of p^2-1 has more than threshold_r smooth bits
}

ulong counter = 0;
double hits = 0;
clock_t duration = 0;

struct sigaction sa_print, sa_alarm;
void print_handler(int sig) {
  printf("#%lu -- %f μs/# -- %e hit rate\n", counter, 1.0 * duration / CLOCKS_PER_SEC, 0.5 * hits / counter);
  printf("Ctrl-C again to quit.\n");
  fflush(stdout);
  alarm(1);
}
void install_handler(int sig) {
  sa_print.sa_handler = print_handler;
  sigemptyset(&sa_print.sa_mask);
  sa_print.sa_flags = SA_RESETHAND; 
  sigaction(SIGINT, &sa_print, NULL);
}

int search(int bits_two, int three_pow, int num_fill, int smoothness, int large_factors, int thread_num) {
  int p_bits = 256, small_samples = 6, p_bits_half = 64;
  bool verbose = false;
  ulong seed;
  // glibc < 2.25 does not have getrandom...
  syscall(SYS_getrandom, &seed, sizeof(ulong), 0);

  sa_alarm.sa_handler = install_handler;
  sigemptyset(&sa_alarm.sa_mask);
  sigaction(SIGALRM, &sa_alarm, NULL);
  install_handler(0);

  //only support for smoothness bounds 2^10 to 2^17
  if (smoothness < 10)
    smoothness = 10;
  if (smoothness > 17)
    smoothness = 17;
  
  std::mt19937_64 prng(seed);

  //write to file
  FILE* results_file;
  string filename = "results/results_p" + to_string(thread_num) + "_" + to_string(seed) + ".txt";
  results_file = fopen(filename.c_str(), "w");

  //printing parameters to output file 
  fprintf(results_file, "## log(p) = %d, B = 2^%d, (2^%d | p±1), pow of 3: %d, %d factors < 2^10, filling with powers of %d, sampling %d extra factors on the large side, thread %d, seed %lu\n",
	 p_bits, smoothness, bits_two, three_pow, small_samples, num_fill, large_factors, thread_num, seed);
  fflush(results_file);
  
  //declare variables used during the search
  mpz_t small, large, large_const, S, L, p, tmp, tmp2, threepow, fivepow, sevenpow, bound, pow2_bound, total_sample;
  mpz_init(small); mpz_init(large); mpz_init(large_const);
  mpz_init(S); mpz_init(L);
  mpz_init(p); mpz_init(tmp); mpz_init(tmp2);
  mpz_init(threepow); mpz_init(fivepow); mpz_init(sevenpow);
  mpz_init(bound); mpz_init(pow2_bound); mpz_init(total_sample);
  mpz_t one, three, five, seven;
  mpz_init_set_ui(one, 1);
  mpz_init_set_ui(three, 3);
  mpz_init_set_ui(five, 5);
  mpz_init_set_ui(seven, 7);
  //int bits, p_bits_r, threshold_r, bits2;
  int k_large1, k_large2;
  bool is_prime, is_smooth;
  


  /************** The "large" side *****************
   *
   * The powers of 2 and 3 do not depend on randomness, so we can compute
   * this factor once and for all.
   */
  mpz_set_ui(large_const, 1);
  mpz_mul_2exp(large_const, large_const, bits_two);
  // Multiply by power of 3
  mpz_pow_ui(threepow, three, three_pow);
  mpz_mul(large_const, large_const, threepow);
  // note: if large_factors = 0, this is a constant,
  // since we don't sample more factors for this side in this case

  //constant for xgcd iterations
  mpz_set_ui(pow2_bound, 1);
  mpz_mul_2exp(pow2_bound, pow2_bound, 127);
  
  


  /************** xgcd-and-boost *****************/


  clock_t last = clock(), now;
  while (true) {
    if (counter  == 1000000) {
      now = clock();
      duration = now - last;
      last = now;
      if (counter == 1 || verbose) {
        printf("#%lu -- %f μs/# -- %e hit rate\n", 1000000 * counter, 1.0 * duration / CLOCKS_PER_SEC, hits / counter);
        fflush(stdout);
      }
    }
    
    /************** Complete the "large" side *****************/
    //sample 0-2 more prime factors for large (excluding 5 and 7, as they are used on the small side)
    mpz_set_ui(total_sample, 1); //keep track of the total size of the sampled factors
    double largelen = 0;
    k_large1 = 0;
    k_large2 = 0;
    mpz_set(large, large_const);

    if (large_factors > 0) {
      k_large1 = prng() % 170 + 3; //keep track of the factors we use on the large side
      mpz_mul_ui(large, large_const, primes[k_large1]);
      largelen += bitlen[k_large1];
    }
    if (large_factors == 2) {
      k_large2 = prng() % 170 + 3; //keep track of the factors we use on the large side
      mpz_mul_ui(large, large, primes[k_large2]);
      largelen += bitlen[k_large2];
    }


    // for each sampled large side, try a couple of small ones
    for (int j = 0; j < 1000000; j++) {  
      counter++;
      /************** The "small" side *****************/
      
      // Choose `small_samples - large_factors` primes (with repetition) from the list of primes < 2^10
      double smalllen = 0;
      mpz_set_ui(small, 1);
      for (int i = 0; i < small_samples - large_factors; i++) {
        int k = prng() % 171 + 2;
        while (k == k_large1 || k == k_large2) {
          k = prng() % 171 + 2;
        }
        mpz_mul_ui(small, small, primes[k]);
        smalllen += bitlen[k];
      }
      
      // Fill up with power of 5 or 7 to get to the right size
      if (num_fill == 7 && floor((p_bits_half - (smalllen + largelen)) / LOG7) > 0) {
        // Add enough powers of 7
        mpz_pow_ui(sevenpow, seven, floor((p_bits_half - (smalllen + largelen)) / LOG7));
        mpz_mul(small, small, sevenpow);
      }
      if (num_fill == 5 && floor((p_bits_half - (smalllen + largelen)) / LOG5) > 0) {
        // Add enough powers of 5
        mpz_pow_ui(fivepow, five, floor((p_bits_half - (smalllen + largelen)) / LOG5));
        mpz_mul(small, small, fivepow);
      }

      /************** XGCD *****************/
      mpz_gcdext(p, S, L, small, large);

      //compute the respective prime: p = 2(|L|*large)^2 - 1
      get_prime(p, L, large, 2, one);
      
      is_prime = mpz_probab_prime_p(p, 1);

      //if p is prime, we check for enough smoothness
      if (is_prime) {
        is_smooth = check_p(p, smoothness, one);
        if (is_smooth) {
          hits++;
          is_prime = mpz_probab_prime_p(p, 4); //make sure p is a prime
          if (is_prime) {
            gmp_fprintf(results_file, "%Zd\n", p); //print to the result file
            fflush(results_file);
          }
        }
      }
      
      //iterate xgcd solutions (see Costello's B-SIDH paper)

      //compute how many iterations we have for getting primes of the correct size
      mpz_mul(tmp, small, large);
      mpz_fdiv_q(bound, pow2_bound, tmp);
      int bound_int = mpz_get_si(bound);
      
      if (bound_int == 0){
        continue;
      }

      // go through positive i
      for (int i = 1; i <= bound_int; i++) {
        mpz_set_ui(p, i);
        mpz_mul(p, small, p); // p = i*small
        mpz_add(p, L, p); //p = L + i*small this is the L for this iteration!

        get_prime(p, p, large, 2, one);

        is_prime = mpz_probab_prime_p(p, 1);

        if (is_prime) {
          
          is_smooth = check_p(p, smoothness, one);

          if (is_smooth) {
            hits++;
            is_prime = mpz_probab_prime_p(p, 4); //make sure p is a prime
            if (is_prime) {
              gmp_fprintf(results_file, "%Zd\n", p);
              fflush(results_file);
            }
          }
        } 
      }

      //go through negative i
      for (int i = 1; i <= bound_int; i++) {
        mpz_set_ui(p, i);
        mpz_mul(p, small, p); // p = i*small
        mpz_sub(p, L, p); //p = L - i*small this is the L for this iteration!

        get_prime(p, p, large, 2, one);

        is_prime = mpz_probab_prime_p(p, 1);

        if (is_prime) {
          
          is_smooth = check_p(p, smoothness, one);

          if (is_smooth) {
            hits++;
            is_prime = mpz_probab_prime_p(p, 4); //make sure p is a prime
            if (is_prime) {
              gmp_fprintf(results_file, "%Zd\n", p);
              fflush(results_file);
            }
          }
        }  
      }
      
    }
    
  }

  mpz_clear(small); mpz_clear(large);
  mpz_clear(S); mpz_clear(L);
  mpz_clear(p); mpz_clear(tmp);
  mpz_clear(threepow); mpz_clear(fivepow);
  mpz_clear(three); mpz_clear(five);

  fclose(results_file);
}

int main(){

  //parameters for search
  //make sure to have enough parameter sets, i.e., >= num_threads/threads_per_run
  int num_threads = 1;
  int threads_per_run = 1;
  
  //exponent of 2 in XGCD input
  int exp_two[] = {32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 40, 40, 32, 32, 33, 33, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 40, 40};

  //exponent of 3 in XGCD input, we want 2^f * 3^e of size roughly 2^64
  int exp_three[] = {20, 20, 19, 19, 19, 19, 18, 18, 17, 17, 17, 17, 16, 16, 15, 15, 20, 20, 19, 19, 19, 19, 18, 18, 17, 17, 17, 17, 16, 16, 15, 15};

  //decide if to fill up the "small" side of XGCD inputs with powers of 5 or 7 up to approriate size
  int filler[] = {5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7, 5, 7};

  //smoothness bound for T (odd smooth factor of p^2-1, must be of size p^(5/4) in SQIsign)
  int smoothness_bound[] = {11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 12};

  //decide if we sample some of the six small ell_i factors for the "large" side (containing powers of 2 and 3)
  //only supports choices 0,1,2
  int large_factors[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

  std::thread threads[num_threads];

  //start threads_per_run threads for each of the parameter sets
  for (int j = 0; j < num_threads/threads_per_run; j++){

    for (int i = 0; i < threads_per_run ; i++) {
        threads[(j * threads_per_run) + i] = std::thread(search, exp_two[j], exp_three[j], filler[j], smoothness_bound[j], large_factors[j], (j * threads_per_run) + i);
    }

  } 
 
  for (int i = 0; i < num_threads; i++) {
      threads[i].join();
  }
  return 0;
}
