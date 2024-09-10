#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED
#include "gmp.h"

#define NOP                     100
#define KAZ_DS_SP_N             "12887130737743415846899404228069312254226272005275462547715916286350895\
                                 94353616859499646611708966470519462094970742052710758498007733709163089\
                                 76819218139972036490209394341226670747650095290054531139398477003815970\
                                 4482003115"
#define KAZ_DS_SP_PHIN          "22832808772359266378099061495397783012893243733591568666589662819258834\
                                 30110688530533015471887040344991123125531686290660633949617119385276010\
                                 83936972281562477380364327107642610764471576454758400000000000000000000\
                                 000000000"
#define KAZ_DS_SP_PHI2N         "26865635941062309238914640074887887671435099564490118224750040682219274\
                                 92183007109584415982062728197252487759883611228245365318290983842052903\
                                 41945173924453132462196017645006952910028800000000000000000000000000000\
                                 00000000"
#define KAZ_DS_SP_G             "77191895919288258297708360801779838039394036329148627807318035471608508\
                                 86138447749690899701565468350755634308814603464884133674962396276027738\
                                 56781377989387985811065667314040888490735410659928651145853516856532510\
                                 073029091"
#define KAZ_DS_SP_ORDERG        "38802000719610065485946728605038635370371669628545225512000"
#define KAZ_DS_SP_PHIORDERG     "4565537405016196669095717123240974201429321318400000000000"
#define KAZ_DS_SP_FAC_ORDERG    32
#define KAZ_DS_SP_R             "18716169074044491239952802950431028031135797097573966038024804367404713\
                                 65015958776000929245280963262152808536212054504187660023264614632924435\
                                 56481219661601494630095666135443498105508350856652404077283594270771011\
                                 688317183"
#define KAZ_DS_SP_ORDERR        "36234549394353990472941141936314128318324539911561538847447640380617441\
                                 35752167128008136294401726015132027071037807723044742124819687889960960\
                                 000000000000000000000000000"
#define KAZ_DS_SP_FAC_ORDERR    17

#define KAZ_DS_ALPHABYTES       26
#define KAZ_DS_VBYTES           94
#define KAZ_DS_S1BYTES          26
#define KAZ_DS_S2BYTES          94
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
