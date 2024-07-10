/*
NIST-developed software is provided by NIST as a public service. You may use, copy, and distribute copies of the software in any medium, provided that you keep intact this entire notice. You may improve, modify, and create derivative works of the software or any portion of the software, and you may copy and distribute such modifications or works. Modified works should carry a notice stating that you changed the software and should note the date and nature of any such change. Please explicitly acknowledge the National Institute of Standards and Technology as the source of the software.
 
NIST-developed software is expressly provided "AS IS." NIST MAKES NO WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT, OR ARISING BY OPERATION OF LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT, AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF, INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY, OR USEFULNESS OF THE SOFTWARE.
 
You are solely responsible for determining the appropriateness of using and distributing the software and you assume all risks associated with its use, including but not limited to the risks and costs of program errors, compliance with applicable laws, damage to or loss of data, programs or equipment, and the unavailability or interruption of operation. This software is not intended to be used in any situation where a failure could cause risk of injury or damage to property. The software developed by NIST employees is not subject to copyright protection within the United States.
*/

//   This is a sample 'api.h' for use 'sign.c'

#ifndef api_h
#define api_h

#define N 128 // 128 bit = 16 bytes

#define d 10 // q^d is the degree as 2^10 = 1024 is the degree of L2 and trapdoor hfe poly G(Z)=Z*l2(Z) has degree 1025
// However P(X) has at most 2^(n-1) degree so infeasible to solve at a first glance

#define PK_ROW_SIZE (N*(N+1))/2
#define VIN_VARS 8

//  Set these three values apropriately for your algorithm
#define CRYPTO_PUBLICKEYBYTES N * PK_ROW_SIZE / 8
#define CRYPTO_SECRETKEYBYTES (3 * N * N + (d+1)*N) / 8 // matrices (T^-1, S^-1, L1^-1) + priv poly G(Z)
#define CRYPTO_BYTES (N + VIN_VARS) / 8 // (x,v) signature tuple 

// Change the algorithm name
#define CRYPTO_ALGNAME "HPPC128"

int
crypto_sign_keypair(unsigned char *pk, unsigned char *sk);

int
crypto_sign(unsigned char *sm, unsigned long long *smlen,
            const unsigned char *m, unsigned long long mlen,
            const unsigned char *sk);

int
crypto_sign_open(unsigned char *m, unsigned long long *mlen,
                 const unsigned char *sm, unsigned long long smlen,
                 const unsigned char *pk);

#endif /* api_h */
