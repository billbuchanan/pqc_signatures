#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED
#include "gmp.h"

#define NOP                     68
#define KAZ_DS_SP_N             "37470874733837919416563211326754079989324849463818175868172713496859968\
                                 4366339106336802166494168058067745412894332797884687187786349732565"
#define KAZ_DS_SP_PHIN          "71467427390759841729059757466289459181369050713019533645557376916391119\
                                 997637068708795979336636964345506399166398464000000000000000000000"
#define KAZ_DS_SP_PHI2N         "89123777242302059588303298495934259673396841021921963245020745403261403\
                                 41255776879891155211825279115447806984192000000000000000000000000"
#define KAZ_DS_SP_G             "37260025342153877976366022431631274000323014010699999970111761875964418\
                                 1266325498418931548345929963501344215735624839833056513259148603733"
#define KAZ_DS_SP_ORDERG        "144070022526464542998162540305862391968000"
#define KAZ_DS_SP_PHIORDERG     "17966317053413597259085197820821504000000"
#define KAZ_DS_SP_FAC_ORDERG    24
#define KAZ_DS_SP_R             "64411872697932469627298024328629501373966883091743836804603337113399879\
                                 222101827567038858750269057628226431053470498775743682787912336229"
#define KAZ_DS_SP_ORDERR        "88155085186093475727706287744104857499274763088175719465155518774692526\
                                 733036565299200000000000000000000"
#define KAZ_DS_SP_FAC_ORDERR    13

#define KAZ_DS_ALPHABYTES       18
#define KAZ_DS_VBYTES           59
#define KAZ_DS_S1BYTES          19
#define KAZ_DS_S2BYTES          58
#define KAZ_DS_SALTBYTES        4


extern void KAZ_DS_OrderBase(mpz_t Modular,
                             mpz_t FiModular,
                             mpz_t Base,
                             mpz_t OrderBase);
extern int KAZ_DS_GET_PFactors(mpz_t input);
extern void KAZ_DS_PFactors(mpz_t ord,
                            mpz_t *pfacs,
                            int *qlist,
                            int *elist);
extern void KAZ_DS_CRT(mpz_t ord,
                       mpz_t *pfactors,
                       mpz_t *candidatex,
                       int nof,
                       mpz_t crt);
extern char* KAZ_DS_MLOG(mpz_t Modular,
                        mpz_t OrderBase,
                        mpz_t Base,
                        mpz_t Target,
                        mpz_t *pfactors,
                        int *qlist,
                        int *elist,
                        int saiz,
                        mpz_t kaz_crt);
extern void KAZ_DS_KeyGen(unsigned char *kaz_ds_verify_key,
                          unsigned char *kaz_ds_sign_key);
extern int KAZ_DS_SIGNATURE(unsigned char *signature,
                             unsigned long long *signlen,
                             const unsigned char *m,
                             unsigned long long mlen,
                             const unsigned char *kaz_ds_sign_key);
extern int KAZ_DS_VERIFICATION(unsigned char *m,
                               unsigned long long *mlen,
                               const unsigned char *sm,
                               unsigned long long smlen,
                               const unsigned char *pk);
extern void KAZ_DS_RANDOM(int lb,
                          int ub,
                          mpz_t out);


#endif // FILE_H_INCLUDED
