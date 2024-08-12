#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "rng.h"
#include "api.h"
#include "gmp.h"
#include "kaz_api.h"
#include "sha256.h"

void HashMsg(const unsigned char *msg, unsigned long long mlen, unsigned char buf[32])
{
    sha256_t hash;
	sha256_init(&hash);
	sha256_update(&hash, msg, mlen);
	sha256_update(&hash, msg, mlen);
	sha256_final(&hash, buf);
}

void KAZ_DS_OrderBase(mpz_t Modular,
                      mpz_t FiModular,
                      mpz_t Base,
                      mpz_t OrderBase)
{
    mpz_t tmp, G1;
    mpz_inits(tmp, G1, NULL);

    int nFiModular=KAZ_DS_GET_PFactors(FiModular);

    mpz_t *pFactors=malloc(nFiModular*sizeof(mpz_t));
    int *q=malloc(nFiModular*sizeof(int));
    int *e=malloc(nFiModular*sizeof(int));
    int *esimpan=malloc(nFiModular*sizeof(int));

    for(int i=0; i<nFiModular; i++) mpz_init(pFactors[i]);
    for(int i=0; i<nFiModular; i++) q[i]=0;
    for(int i=0; i<nFiModular; i++) e[i]=0;
    for(int i=0; i<nFiModular; i++) esimpan[i]=0;

    KAZ_DS_PFactors(FiModular, pFactors, q, e);

    for(int j=0; j<nFiModular; j++){
        for(int i=1; i<=e[j]; i++){
            mpz_set_ui(tmp, q[j]);
            mpz_pow_ui(tmp, tmp, i);
            mpz_divexact(tmp, FiModular, tmp);
            mpz_powm(tmp, Base, tmp, Modular);

            if(mpz_cmp_ui(tmp, 1)!=0){
                esimpan[j]=i-1;
                break;
            }
        }
    }

    mpz_set_ui(G1, 1);
    for(int j=0; j<nFiModular; j++){
        mpz_set_ui(tmp, q[j]);
        mpz_pow_ui(tmp, tmp, esimpan[j]);
        mpz_mul(G1, G1, tmp);
    }

    mpz_divexact(OrderBase, FiModular, G1);
}

void KAZ_DS_CRT(mpz_t ord,
                mpz_t *pfactors,
                mpz_t *candidatex,
                int nof,
                mpz_t crt)
{
    mpz_t tmp;
    mpz_init(tmp);

    mpz_t *b=malloc(nof*sizeof(mpz_t));
    mpz_t *binv=malloc(nof*sizeof(mpz_t));

    for(int i=0; i<nof; i++) mpz_init(b[i]);
    for(int i=0; i<nof; i++) mpz_init(binv[i]);
    for(int i=0; i<nof; i++) mpz_divexact(b[i], ord, pfactors[i]);
    for(int i=0; i<nof; i++){
        mpz_set(tmp, pfactors[i]);
        mpz_invert(binv[i], b[i], tmp);
    }

    mpz_set_ui(crt, 0);

    for(int i=0; i<nof; i++){
        mpz_set(tmp, candidatex[i]);
        mpz_mul(tmp, tmp, b[i]);
        mpz_mul(tmp, tmp, binv[i]);
        mpz_mod(tmp, tmp, ord);
        mpz_add(crt, crt, tmp);
        mpz_mod(crt, crt, ord);
    }
}

int KAZ_DS_GET_PFactors(mpz_t input)
{
    mpz_t inp, prod;
    mpz_inits(inp, prod, NULL);
    mpz_set(inp, input);
    mpz_set_ui(prod, 1);

    int div=2, count=0;
    int i=0;

    while(mpz_cmp_ui(inp, 1)>0){
        while(mpz_divisible_ui_p(inp, div)>0){
            count++;
            mpz_divexact_ui(inp, inp, div);
            mpz_mul_ui(prod, prod, div);
        }

        if(mpz_cmp_ui(prod, 1)>0){
            i++;
        }
        mpz_set_ui(prod, 1);
        div++;
        count=0;
    }

    return i;
}

void KAZ_DS_PFactors(mpz_t ord,
                     mpz_t *pfacs,
                     int *qlist,
                     int *elist)
{
    mpz_t inp, prod;
    mpz_inits(inp, prod, NULL);
    mpz_set(inp, ord);
    mpz_set_ui(prod, 1);

    int div=2, count=0;
    unsigned long long i=0;

    while(mpz_cmp_ui(inp, 1)>0){
        while(mpz_divisible_ui_p(inp, div)>0){
            count++;
            mpz_divexact_ui(inp, inp, div);
            mpz_mul_ui(prod, prod, div);
        }

        if(mpz_cmp_ui(prod, 1)>0){

            mpz_set(pfacs[i], prod);
            qlist[i]=div;
            elist[i]=count;
            i++;
        }
        mpz_set_ui(prod, 1);
        div++;
        count=0;
    }

    mpz_clear(inp);
}

char* KAZ_DS_MLOG(mpz_t n,
                 mpz_t ord,
                 mpz_t g,
                 mpz_t h,
                 mpz_t *pfactors,
                 int *qlist,
                 int *elist,
                 int saiz,
                 mpz_t kaz_crt)
{
    mpz_t tmp, tmp2, exp, G, H, j, delta, alpha, beta;
    mpz_inits(tmp, tmp2, exp, G, H, j, delta, alpha, beta, NULL);

    unsigned long kk=0;
    mpz_set_ui(kaz_crt, 0);

    int SAIZ=saiz;

    mpz_t *candidateX=malloc(SAIZ*sizeof(mpz_t));
    for(int i=0; i<SAIZ; i++) mpz_init(candidateX[i]);
    for(int i=0; i<SAIZ; i++) mpz_set_ui(candidateX[i], 0);
    for(int i=SAIZ-1; i>=0; i--){
        mpz_set_ui(delta, 1);
        kk=0;
        mpz_t *l=malloc(elist[i]*sizeof(mpz_t));

        for(int ii=0; ii<elist[i]; ii++) mpz_init(l[ii]);
        for(int j=1; j<=elist[i]; j++){
            mpz_divexact_ui(tmp, ord, qlist[i]);
            mpz_powm(alpha, g, tmp, n);

            if(elist[i]==1){
                mpz_powm(beta, h, tmp, n);
                mpz_t k, limits;
                mpz_inits(k, limits, NULL);
                mpz_set_ui(k, 0);
                mpz_set_ui(limits, qlist[i]);
                mpz_pow_ui(limits, limits, j);
                mpz_sub_ui(limits, limits, 1);

                while(mpz_cmp(k, limits)<1){
                    mpz_t tmp3; mpz_init(tmp3);
                    mpz_powm(tmp3, alpha, k, n);

                    if(mpz_cmp(tmp3, beta)==0){
                        break;
                    }
                    if(mpz_cmp(k, limits)>=0){
                        return "FAIL";
                    }
                    mpz_add_ui(k, k, 1);
                }

                mpz_set(candidateX[i], k);
            }else{
                mpz_t tmp5;
                mpz_init(tmp5);

                mpz_set_ui(tmp, qlist[i]);
                mpz_pow_ui(tmp, tmp, j); //q[i]^j
                mpz_divexact(tmp, ord, tmp); //n/q[i]^j
                mpz_powm(beta, h, tmp, n);
                mpz_neg(tmp, tmp);
                mpz_powm(tmp5, delta, tmp, n);
                mpz_mul(beta, beta, tmp5);
                mpz_mod(beta, beta, n);

                mpz_t k, limits;
                mpz_inits(k, limits, NULL);

                mpz_set_ui(k, 0);
                mpz_set_ui(limits, qlist[i]);
                mpz_pow_ui(limits, limits, j);
                mpz_sub_ui(limits, limits, 1);

                while(mpz_cmp(k, limits)<1){
                    mpz_t tmp3; mpz_init(tmp3);
                    mpz_powm(tmp3, alpha, k, n);

                    if(mpz_cmp(tmp3, beta)==0){
                        break;
                    }
                    if(mpz_cmp(k, limits)>=0){
                        return "FAIL";
                    }
                    mpz_add_ui(k, k, 1);
                }

                mpz_set(l[kk], k);

                mpz_t pwr, tmp4, tmp6;
                mpz_inits(pwr, tmp4, tmp6, NULL);

                mpz_set_ui(pwr, qlist[i]);
                mpz_pow_ui(pwr, pwr, (j-1));
                mpz_mul(pwr, l[kk], pwr);
                mpz_powm(tmp4, g, pwr, n);
                mpz_mul(delta, delta, tmp4);
                mpz_mod(delta, delta, n);
                mpz_add(candidateX[i], candidateX[i], pwr);
                kk++;
            }
        }

        mpz_mod(candidateX[i], candidateX[i], pfactors[i]);
    }

    int k = SAIZ;
    KAZ_DS_CRT(ord, pfactors, candidateX, k, kaz_crt);
}

void KAZ_DS_KeyGen(unsigned char *kaz_ds_verify_key,
                   unsigned char *kaz_ds_sign_key)
{
    mpz_t N, PHIN, PHI2N, g, Gg, PHIGg, R, GR, tmp, ALPHA, V, v, z2;
    mpz_inits(N, PHIN, PHI2N, g, Gg, PHIGg, R, GR, tmp, ALPHA, V, v, z2, NULL);

    //1) Get all system parameters
    mpz_set_str(N, KAZ_DS_SP_N, 10);
    mpz_set_str(PHIN, KAZ_DS_SP_PHIN, 10);
    mpz_set_str(PHI2N, KAZ_DS_SP_PHI2N, 10);
    mpz_set_str(g, KAZ_DS_SP_G, 10);
    mpz_set_str(Gg, KAZ_DS_SP_ORDERG, 10);
    mpz_set_str(PHIGg, KAZ_DS_SP_PHIORDERG, 10);
    mpz_set_str(R, KAZ_DS_SP_R, 10);
    mpz_set_str(GR, KAZ_DS_SP_ORDERR, 10);

    int n=mpz_sizeinbase(N, 2);
    int nphiGg=mpz_sizeinbase(PHIGg, 2);

    //2) Generate ALPHA & V
    mpz_t *primefactor=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(mpz_t));
    int *q=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(int));
    int *e=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(int));

    for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) mpz_init(primefactor[i]);
    for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) q[i]=0;
    for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) e[i]=0;

    KAZ_DS_PFactors(Gg, primefactor, q, e);

    char *ret;

    do{
        KAZ_DS_RANDOM(nphiGg-2, nphiGg-1, ALPHA);

        mpz_powm(tmp, R, ALPHA, PHIN);
        mpz_powm(V, g, tmp, N);
        KAZ_DS_MLOG(N, Gg, g, V, primefactor, q, e, KAZ_DS_SP_FAC_ORDERG, v);

        primefactor=malloc(KAZ_DS_SP_FAC_ORDERR*sizeof(mpz_t));
        q=malloc(KAZ_DS_SP_FAC_ORDERR*sizeof(int));
        e=malloc(KAZ_DS_SP_FAC_ORDERR*sizeof(int));

        for(int i=0; i<KAZ_DS_SP_FAC_ORDERR; i++) mpz_init(primefactor[i]);
        for(int i=0; i<KAZ_DS_SP_FAC_ORDERR; i++) q[i]=0;
        for(int i=0; i<KAZ_DS_SP_FAC_ORDERR; i++) e[i]=0;

        KAZ_DS_PFactors(GR, primefactor, q, e);
        ret=KAZ_DS_MLOG(PHIN, GR, R, v, primefactor, q, e, KAZ_DS_SP_FAC_ORDERR, z2);
    }while(mpz_cmp(v, tmp)==0 || ret!="FAIL");

    //3) Set kaz_ds_sign_key=(ALPHA, V) & kaz_ds_verify_key=V
    size_t ALPHASIZE=mpz_sizeinbase(ALPHA, 16);
	size_t VSIZE=mpz_sizeinbase(V, 16);

	unsigned char *ALPHABYTE=(unsigned char*) malloc(ALPHASIZE*sizeof(unsigned char));
	mpz_export(ALPHABYTE, &ALPHASIZE, 1, sizeof(char), 0, 0, ALPHA);

	unsigned char *VBYTE=(unsigned char*) malloc(VSIZE*sizeof(unsigned char));
	mpz_export(VBYTE, &VSIZE, 1, sizeof(char), 0, 0, V);

	for(int i=0; i<CRYPTO_SECRETKEYBYTES; i++) kaz_ds_sign_key[i]=32;

	int je=CRYPTO_SECRETKEYBYTES-1;
	for(int i=VSIZE-1; i>=0; i--){
		kaz_ds_sign_key[je]=VBYTE[i];
		je--;
	}

	je=CRYPTO_SECRETKEYBYTES-KAZ_DS_VBYTES-1;
	for(int i=ALPHASIZE-1; i>=0; i--){
		kaz_ds_sign_key[je]=ALPHABYTE[i];
		je--;
	}

	for(int i=0; i<CRYPTO_PUBLICKEYBYTES; i++) kaz_ds_verify_key[i]=32;

	je=CRYPTO_PUBLICKEYBYTES-1;
	for(int i=VSIZE-1; i>=0; i--){
		kaz_ds_verify_key[je]=VBYTE[i];
		je--;
	}

	//printf("Lepas Sini KEYEGN\n");
	mpz_clears(N, PHIN, PHI2N, g, Gg, PHIGg, R, GR, tmp, ALPHA, V, v, z2, NULL);
}

int KAZ_DS_SIGNATURE(unsigned char *signature,
                      unsigned long long *signlen,
                      const unsigned char *m,
                      unsigned long long mlen,
                      const unsigned char *sk)
{
    mpz_t N, PHIN, PHI2N, g, Gg, PHIGg, R, GR, tmp, ALPHA, V, hashValue, r, S0, S1, S2, z2, S2F;
    mpz_t v, GS1, GRGg, aF, w0, w1, w2, SALT;
    mpz_inits(N, PHIN, PHI2N, g, Gg, PHIGg, R, GR, tmp, ALPHA, V, hashValue, r, S0, S1, S2, z2, S2F, NULL);
    mpz_inits(v, GS1, GRGg, aF, w0, w1, w2, SALT, NULL);

    //1) Get all system parameters
    mpz_set_str(N, KAZ_DS_SP_N, 10);
    mpz_set_str(PHIN, KAZ_DS_SP_PHIN, 10);
    mpz_set_str(PHI2N, KAZ_DS_SP_PHI2N, 10);
    mpz_set_str(g, KAZ_DS_SP_G, 10);
    mpz_set_str(Gg, KAZ_DS_SP_ORDERG, 10);
    mpz_set_str(PHIGg, KAZ_DS_SP_PHIORDERG, 10);
    mpz_set_str(R, KAZ_DS_SP_R, 10);
    mpz_set_str(GR, KAZ_DS_SP_ORDERR, 10);

    int n=mpz_sizeinbase(N, 2);
    int nphiGg=mpz_sizeinbase(PHIGg, 2);

    //2) Get kaz_ds_sign_key=(ALPHA, V)
    int ALPHASIZE=0;
	for(int i=0; i<KAZ_DS_ALPHABYTES; i++){
        if((int)sk[i]==32) ALPHASIZE++;
        else break;
	}

	int VSIZE=0;
	for(int i=KAZ_DS_ALPHABYTES; i<CRYPTO_SECRETKEYBYTES; i++){
        if((int)sk[i]==32) VSIZE++;
        else break;
	}
	unsigned char *ALPHABYTE=(unsigned char*) malloc((KAZ_DS_ALPHABYTES-ALPHASIZE)*sizeof(unsigned char));
	unsigned char *VBYTE=(unsigned char*) malloc((KAZ_DS_VBYTES-VSIZE)*sizeof(unsigned char));

	for(int i=0; i<KAZ_DS_ALPHABYTES-ALPHASIZE; i++) ALPHABYTE[i]=32;
	for(int i=0; i<KAZ_DS_VBYTES-VSIZE; i++) VBYTE[i]=32;

	for(int i=0; i<KAZ_DS_ALPHABYTES-ALPHASIZE; i++){ALPHABYTE[i]=sk[i+ALPHASIZE];}
	for(int i=0; i<KAZ_DS_VBYTES-VSIZE; i++){VBYTE[i]=sk[i+(KAZ_DS_ALPHABYTES+VSIZE)];}

    mpz_import(ALPHA, KAZ_DS_ALPHABYTES-ALPHASIZE, 1, sizeof(char), 0, 0, ALPHABYTE);
	mpz_import(V, KAZ_DS_VBYTES-VSIZE, 1, sizeof(char), 0, 0, VBYTE);

	//3) Generate SALT and Compute the hash value of the message||SALT
	unsigned char* mSALT=(unsigned char*) malloc((mlen+KAZ_DS_SALTBYTES)*sizeof(unsigned char));
    unsigned char* SALTBYTE=(unsigned char*) malloc(KAZ_DS_SALTBYTES*sizeof(unsigned char));

	do{
	    do{
            randombytes(SALTBYTE, 4);

            for(int i=0; i<mlen+KAZ_DS_SALTBYTES; i++) mSALT[i]=0;
            for(int i=0; i<mlen; i++) mSALT[i]=m[i];
            int j=0;
            for(int i=mlen; i<mlen+KAZ_DS_SALTBYTES; i++){
                mSALT[i]=SALTBYTE[j];
                j++;
            }

            unsigned char buf[CRYPTO_BYTES]={0};
            HashMsg(mSALT, mlen+KAZ_DS_SALTBYTES, buf);

            mpz_import(hashValue, CRYPTO_BYTES, 1, sizeof(char), 0, 0, buf);

            //4) Generate S0, S1, S2

            mpz_t *primefactor=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(mpz_t));
            int *q=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(int));
            int *e=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(int));

            for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) mpz_init(primefactor[i]);
            for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) q[i]=0;
            for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) e[i]=0;

            KAZ_DS_PFactors(Gg, primefactor, q, e);
            do{
                char *ret;

                do{
                    KAZ_DS_RANDOM(nphiGg-2, nphiGg-1, r);
                    mpz_nextprime(r, r);

                    mpz_powm(tmp, R, r, PHIN);
                    mpz_powm(S0, g, tmp, N);
                    KAZ_DS_MLOG(N, Gg, g, S0, primefactor, q, e, KAZ_DS_SP_FAC_ORDERG, S1);

                    primefactor=malloc(KAZ_DS_SP_FAC_ORDERR*sizeof(mpz_t));
                    q=malloc(KAZ_DS_SP_FAC_ORDERR*sizeof(int));
                    e=malloc(KAZ_DS_SP_FAC_ORDERR*sizeof(int));

                    for(int i=0; i<KAZ_DS_SP_FAC_ORDERR; i++) mpz_init(primefactor[i]);
                    for(int i=0; i<KAZ_DS_SP_FAC_ORDERR; i++) q[i]=0;
                    for(int i=0; i<KAZ_DS_SP_FAC_ORDERR; i++) e[i]=0;

                    KAZ_DS_PFactors(GR, primefactor, q, e);
                    ret=KAZ_DS_MLOG(PHIN, GR, R, S1, primefactor, q, e, KAZ_DS_SP_FAC_ORDERR, z2);
                }while(mpz_cmp(S1, tmp)==0 || ret!="FAIL");

                mpz_add(S2, hashValue, ALPHA);
                mpz_mod(S2, S2, PHI2N);
                mpz_invert(tmp, r, PHI2N);
                mpz_mul(S2, S2, tmp);
                mpz_mod(S2, S2, PHI2N);

                /******************* START CHECKING PURPOSES ONLY *******************/
                mpz_set_ui(S2F, 0);

                primefactor=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(mpz_t));
                q=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(int));
                e=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(int));

                for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) mpz_init(primefactor[i]);
                for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) q[i]=0;
                for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) e[i]=0;

                KAZ_DS_PFactors(Gg, primefactor, q, e);
                KAZ_DS_MLOG(N, Gg, g, V, primefactor, q, e, KAZ_DS_SP_FAC_ORDERG, v);

                mpz_powm(tmp, R, hashValue, PHIN);
                mpz_mul(tmp, v, tmp);
                mpz_mod(tmp, tmp, PHIN);

                KAZ_DS_OrderBase(PHIN, PHI2N, S1, GS1);
                int nGS1=KAZ_DS_GET_PFactors(GS1);

                primefactor=malloc(nGS1*sizeof(mpz_t));
                q=malloc(nGS1*sizeof(int));
                e=malloc(nGS1*sizeof(int));

                for(int i=0; i<nGS1; i++) mpz_init(primefactor[i]);
                for(int i=0; i<nGS1; i++) q[i]=0;
                for(int i=0; i<nGS1; i++) e[i]=0;

                KAZ_DS_PFactors(GS1, primefactor, q, e);
                ret=KAZ_DS_MLOG(PHIN, GS1, S1, tmp, primefactor, q, e, nGS1, S2F);
            }while(mpz_cmp(S2, S2F)==0);//printf("Lepas Sini mpz_cmp(S2, S2F)\n");

            KAZ_DS_OrderBase(Gg, PHIGg, R, GRGg);
            int nGRGg=KAZ_DS_GET_PFactors(GRGg);

            primefactor=malloc(nGRGg*sizeof(mpz_t));
            q=malloc(nGRGg*sizeof(int));
            e=malloc(nGRGg*sizeof(int));

            for(int i=0; i<nGRGg; i++) mpz_init(primefactor[i]);
            for(int i=0; i<nGRGg; i++) q[i]=0;
            for(int i=0; i<nGRGg; i++) e[i]=0;

            KAZ_DS_PFactors(GRGg, primefactor, q, e);
            KAZ_DS_MLOG(Gg, GRGg, R, v, primefactor, q, e, nGRGg, aF);

            mpz_invert(tmp, S2, PHI2N);//gmp_printf("mpz_invert(tmp, S2, PHI2N)=%Zd\n", tmp);
        }while(mpz_cmp_ui(tmp, 0)==0);//printf("Lepas Sini mpz_cmp_ui(tmp, 0)\n");

        mpz_add(w0, aF, hashValue);
        mpz_mul(w0, w0, tmp);
        mpz_mod(w0, w0, PHI2N);

        mpz_powm(w1, g, S1, N);

        mpz_powm(tmp, R, w0, PHIN);
        mpz_powm(w2, g, tmp, N);//gmp_printf("mpz_cmp_ui(tmp, 0)=%d\n", mpz_cmp_ui(tmp, 0));
    }while(mpz_cmp(w1, w2)==0);//printf("Lepas Sini mpz_cmp(w1, w2)\n");

    //5) Set signature=(S1, S2)
    size_t S1SIZE=mpz_sizeinbase(S1, 16);
	size_t S2SIZE=mpz_sizeinbase(S2, 16);//printf("KAZ_DS_SALTBYTES=%d\n", KAZ_DS_SALTBYTES);

    for(int i=0; i<mlen+KAZ_DS_SALTBYTES+KAZ_DS_S1BYTES+KAZ_DS_S2BYTES; i++) signature[i]=32;

	unsigned char *S1BYTE=(unsigned char*) malloc(S1SIZE*sizeof(unsigned char));
	mpz_export(S1BYTE, &S1SIZE, 1, sizeof(char), 0, 0, S1);

	unsigned char *S2BYTE=(unsigned char*) malloc(S2SIZE*sizeof(unsigned char));
	mpz_export(S2BYTE, &S2SIZE, 1, sizeof(char), 0, 0, S2);

	int je=KAZ_DS_SALTBYTES+mlen+KAZ_DS_S1BYTES+KAZ_DS_S2BYTES-1;
	for(int i=KAZ_DS_SALTBYTES+mlen-1; i>=0; i--){
		signature[je]=mSALT[i];
		je--;
	}

	/*je=mlen+KAZ_DS_S1BYTES+KAZ_DS_S2BYTES-1;
	for(int i=mlen-1; i>=0; i--){
		signature[je]=m[i];
		je--;
	}*/

	je=KAZ_DS_S1BYTES+KAZ_DS_S2BYTES-1;
	for(int i=S2SIZE-1; i>=0; i--){
		signature[je]=S2BYTE[i];
		je--;
	}

    je=KAZ_DS_S1BYTES-1;
	for(int i=S1SIZE-1; i>=0; i--){
		signature[je]=S1BYTE[i];
		je--;
	}

	*signlen=KAZ_DS_SALTBYTES+mlen+KAZ_DS_S1BYTES+KAZ_DS_S2BYTES;
    //gmp_printf("mlen=%d\n", mlen);
	//gmp_printf("mlen+KAZ_DS_S1BYTES+KAZ_DS_S2BYTES=%d\n", (mlen+KAZ_DS_S1BYTES+KAZ_DS_S2BYTES));

    free(S1BYTE);
    free(S2BYTE);
	mpz_clears(N, PHIN, PHI2N, g, Gg, PHIGg, R, GR, tmp, ALPHA, V, hashValue, r, S0, S1, S2, z2, S2F, NULL);
    mpz_clears(v, GS1, GRGg, aF, w0, w1, w2, SALT, NULL);
    //printf("Lepas Sini\n");
    return 0;
}

int KAZ_DS_VERIFICATION(unsigned char *m,
                        unsigned long long *mlen,
                        const unsigned char *sm,
                        unsigned long long smlen,
                        const unsigned char *pk)
{
    mpz_t N, PHIN, PHI2N, g, Gg, PHIGg, R, GR, tmp, V, S1, S2, hashValue, S2F, v, GS1, GRGg, aF;
    mpz_t w0, w1, w2, y1, y2, SALT;
    mpz_inits(N, PHIN, PHI2N, g, Gg, PHIGg, R, GR, tmp, V, S1, S2, hashValue, S2F, v, GS1, GRGg, aF, NULL);
    mpz_inits(w0, w1, w2, y1, y2, SALT, NULL);

    //1) Get all system parameters
    mpz_set_str(N, KAZ_DS_SP_N, 10);
    mpz_set_str(PHIN, KAZ_DS_SP_PHIN, 10);
    mpz_set_str(PHI2N, KAZ_DS_SP_PHI2N, 10);
    mpz_set_str(g, KAZ_DS_SP_G, 10);
    mpz_set_str(Gg, KAZ_DS_SP_ORDERG, 10);
    mpz_set_str(PHIGg, KAZ_DS_SP_PHIORDERG, 10);
    mpz_set_str(R, KAZ_DS_SP_R, 10);
    mpz_set_str(GR, KAZ_DS_SP_ORDERR, 10);

    int n=mpz_sizeinbase(N, 2);
    int nphiGg=mpz_sizeinbase(PHIGg, 2);

    //2) Get kaz_ds_verify_key=(V)
	int VSIZE=0;
	for(int i=0; i<CRYPTO_PUBLICKEYBYTES; i++){
        if((int)pk[i]==32) VSIZE++;
        else break;
	}

	unsigned char *VBYTE=(unsigned char*) malloc((KAZ_DS_VBYTES-VSIZE)*sizeof(unsigned char));

	for(int i=0; i<KAZ_DS_VBYTES-VSIZE; i++) VBYTE[i]=32;
	for(int i=0; i<KAZ_DS_VBYTES-VSIZE; i++){VBYTE[i]=pk[i+VSIZE];}

	mpz_import(V, KAZ_DS_VBYTES-VSIZE, 1, sizeof(char), 0, 0, VBYTE);

    //3) Get signature=(S1, S2, m, SALT)
    int S1SIZE=0;
	for(int i=0; i<KAZ_DS_S1BYTES; i++){
        if((int)sm[i]==32) S1SIZE++;
        else break;
	}

	int S2SIZE=0;
	for(int i=KAZ_DS_S1BYTES; i<KAZ_DS_S2BYTES; i++){
        if((int)sm[i]==32) S2SIZE++;
        else break;
	}

	int MSIZE=0;
	for(int i=KAZ_DS_S1BYTES+KAZ_DS_S2BYTES; i<smlen; i++){
        if((int)sm[i]==32) MSIZE++;
        else break;
	}

	unsigned char *S1BYTE=(unsigned char*) malloc((KAZ_DS_S1BYTES-S1SIZE)*sizeof(unsigned char));
	unsigned char *S2BYTE=(unsigned char*) malloc((KAZ_DS_S2BYTES-S2SIZE)*sizeof(unsigned char));
	unsigned char *MBYTE=(unsigned char*) malloc((smlen-(KAZ_DS_S1BYTES+KAZ_DS_S2BYTES)-MSIZE)*sizeof(unsigned char));

	for(int i=0; i<KAZ_DS_S1BYTES-S1SIZE; i++) S1BYTE[i]=32;
	for(int i=0; i<KAZ_DS_S2BYTES-S2SIZE; i++) S2BYTE[i]=32;
	for(int i=0; i<smlen-(KAZ_DS_S1BYTES+KAZ_DS_S2BYTES)-MSIZE; i++) MBYTE[i]=32;

	for(int i=0; i<KAZ_DS_S1BYTES-S1SIZE; i++){S1BYTE[i]=sm[i+S1SIZE];}
	for(int i=0; i<KAZ_DS_S2BYTES-S2SIZE; i++){S2BYTE[i]=sm[i+(KAZ_DS_S1BYTES+S2SIZE)];}
	for(int i=0; i<smlen-(KAZ_DS_S1BYTES+KAZ_DS_S2BYTES)-MSIZE; i++){MBYTE[i]=sm[i+(KAZ_DS_S1BYTES+KAZ_DS_S2BYTES+MSIZE)];}

    mpz_import(S1, KAZ_DS_S1BYTES-S1SIZE, 1, sizeof(char), 0, 0, S1BYTE);
	mpz_import(S2, KAZ_DS_S2BYTES-S2SIZE, 1, sizeof(char), 0, 0, S2BYTE);

	//4) Compute the hash value of the message
    unsigned char buf[CRYPTO_BYTES]={0}; int len=smlen-KAZ_DS_S1BYTES-KAZ_DS_S2BYTES-MSIZE;
    HashMsg(MBYTE, len, buf);

    mpz_import(hashValue, CRYPTO_BYTES, 1, sizeof(char), 0, 0, buf);

    /******************* START CHECKING PURPOSES ONLY *******************/
    mpz_set_ui(S2F, 0);

    mpz_t *primefactor=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(mpz_t));
    int *q=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(int));
    int *e=malloc(KAZ_DS_SP_FAC_ORDERG*sizeof(int));

    for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) mpz_init(primefactor[i]);
    for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) q[i]=0;
    for(int i=0; i<KAZ_DS_SP_FAC_ORDERG; i++) e[i]=0;

    KAZ_DS_PFactors(Gg, primefactor, q, e);
    KAZ_DS_MLOG(N, Gg, g, V, primefactor, q, e, KAZ_DS_SP_FAC_ORDERG, v);

    mpz_powm(tmp, R, hashValue, PHIN);
    mpz_mul(tmp, v, tmp);
    mpz_mod(tmp, tmp, PHIN);

    KAZ_DS_OrderBase(PHIN, PHI2N, S1, GS1);
    int nGS1=KAZ_DS_GET_PFactors(GS1);

    primefactor=malloc(nGS1*sizeof(mpz_t));
    q=malloc(nGS1*sizeof(int));
    e=malloc(nGS1*sizeof(int));

    for(int i=0; i<nGS1; i++) mpz_init(primefactor[i]);
    for(int i=0; i<nGS1; i++) q[i]=0;
    for(int i=0; i<nGS1; i++) e[i]=0;

    KAZ_DS_PFactors(GS1, primefactor, q, e);
    char *ret=KAZ_DS_MLOG(PHIN, GS1, S1, tmp, primefactor, q, e, nGS1, S2F);

    if(mpz_cmp(S2, S2F)==0){
        //printf("Reject Signature mpz_cmp(S2, S2F)\n");
        return -4; //REJECT SIGNATURE
    }

    KAZ_DS_OrderBase(Gg, PHIGg, R, GRGg);
    int nGRGg=KAZ_DS_GET_PFactors(GRGg);

    primefactor=malloc(nGRGg*sizeof(mpz_t));
    q=malloc(nGRGg*sizeof(int));
    e=malloc(nGRGg*sizeof(int));

    for(int i=0; i<nGRGg; i++) mpz_init(primefactor[i]);
    for(int i=0; i<nGRGg; i++) q[i]=0;
    for(int i=0; i<nGRGg; i++) e[i]=0;

    KAZ_DS_PFactors(GRGg, primefactor, q, e);
    KAZ_DS_MLOG(Gg, GRGg, R, v, primefactor, q, e, nGRGg, aF);

    mpz_invert(tmp, S2, PHI2N);

    if(mpz_cmp_ui(tmp, 0)==0) return -2; //ABORT SIGNING

    mpz_add(w0, aF, hashValue);
    mpz_mul(w0, w0, tmp);
    mpz_mod(w0, w0, PHI2N);

    mpz_powm(w1, g, S1, N);

    mpz_powm(tmp, R, w0, PHIN);
    mpz_powm(w2, g, tmp, N);

    if(mpz_cmp(w1, w2)==0){
        //printf("Reject Signature mpz_cmp(w1, w2)\n");
        return -4; //REJECT SIGNATURE
    }
//gmp_printf("tmp=%Zd\n", tmp);
//printf("Checking Successed\n");
    mpz_powm(tmp, S1, S2, PHIN);
	mpz_powm(y1, g, tmp, N);

	mpz_powm(tmp, R, hashValue, PHIN);
	mpz_powm(y2, V, tmp, N);

	//mpz_clears(N, PHIN, PHI2N, g, Gg, PHIGg, R, GR, tmp, V, S1, S2, hashValue, S2F, v, GS1, GRGg, aF);
    //mpz_clears(w0, w1, w2);

	if(mpz_cmp(y1, y2)==0){
        memcpy(m, MBYTE, len-KAZ_DS_SALTBYTES);
        *mlen=len-KAZ_DS_SALTBYTES;

        free(S1BYTE);
        free(S2BYTE);
        free(MBYTE);
        return 0;
	}

	//mpz_clears(y1, y2);

	return -4;
}

void KAZ_DS_RANDOM(int lb,
                   int ub,
                   mpz_t out){
    ////time_t seed;
	//srand((unsigned) time(&seed));

	//mpz_t randNUM; mpz_init(randNUM);

	mpz_t lbound, ubound;

	gmp_randstate_t gmpRandState;
    gmp_randinit_mt(gmpRandState);
	mpz_inits(lbound, ubound, NULL);

	mpz_ui_pow_ui(lbound, 2, lb);
	mpz_ui_pow_ui(ubound, 2, ub);

	unsigned int sd=100000;

	do{
		// initialize state for a Mersenne Twister algorithm. This algorithm is fast and has good randomness properties.

		//gmp_randseed_ui(gmpRandState, sd);
		gmp_randseed_ui(gmpRandState, rand()+sd);
		mpz_urandomb(out, gmpRandState, ub);
		sd+=1;
	}while((mpz_cmp(out, lbound) == -1) || (mpz_cmp(out, ubound) == 1));

	// empty the memory location for the random generator state
    //gmp_randclear(gmpRandState);
}

/*void KAZ_DS_RANDOM(int lb,
                   int ub,
                   mpz_t out){
    time_t seed;
	srand((unsigned) time(&seed));

	mpz_t randNUM; mpz_init(randNUM);

	mpz_t lbound, ubound;

	gmp_randstate_t gmpRandState;

	mpz_inits(lbound, ubound, NULL);

	mpz_ui_pow_ui(lbound, 2, lb);
	mpz_ui_pow_ui(ubound, 2, ub);

	do{
		// initialize state for a Mersenne Twister algorithm. This algorithm is fast and has good randomness properties.
		gmp_randinit_mt(gmpRandState);
		gmp_randseed_ui(gmpRandState, rand());
		mpz_urandomb(out, gmpRandState, ub);
	}while((mpz_cmp(out, lbound) == -1) || (mpz_cmp(out, ubound) == 1));

	// empty the memory location for the random generator state
    gmp_randclear(gmpRandState);
}*/
