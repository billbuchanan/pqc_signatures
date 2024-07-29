#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED
#include "gmp.h"

#define NOP                     125
#define KAZ_DS_SP_N             "97338865350576886716107547998119626182298756390902079429186037673784277\
                                 02701564139578352984230854289084319163516017271342432460067909493522405\
                                 07065550432794900760361038625760604692536277050160652728009280184779283\
                                 01326420767108423981286225716812490063801072697529350986324293180613727\
                                 42686715"
#define KAZ_DS_SP_PHIN          "16566626491957825195560524633368338213617082385176322508656508271996453\
                                 44639254945253387320371724755983318616677478847844199120273990458816615\
                                 33168594898375354177450303114495197392219689850571554481340831664768156\
                                 00472495888502971757601180371567293497344000000000000000000000000000000\
                                 00000000"
#define KAZ_DS_SP_PHI2N         "18290848715264049800149742321160696710772352715941304565417444777038688\
                                 15560345145197338898472418032937321760843894650298352207927006191666450\
                                 85413029330253881155096427852107436577558751908209202113702136704289282\
                                 10975315760938374829881425920000000000000000000000000000000000000000000\
                                 0000000"
#define KAZ_DS_SP_G             "88626006325303055006228230283380679024074188035285475142809326571530676\
                                 27739751166299832411387629174021014151059117683759836939632505724130347\
                                 26140644551866083075123711358991731606891875644399401207071227611598512\
                                 24207741950539422831866154206033840194751669774987944546271606396944276\
                                 64326669"
#define KAZ_DS_SP_ORDERG        "99154693887499828557116081873795155652147461554242228686027806044656980\
                                 768000"
#define KAZ_DS_SP_PHIORDERG     "10947452133269193990830886255076544097435968939032535969562624000000000\
                                 000000"
#define KAZ_DS_SP_FAC_ORDERG    40
#define KAZ_DS_SP_R             "15360178485604580394368272462069523466687262393165912627843218900053671\
                                 09083746371859970873868469622678575813025432017171320586957461472002874\
                                 92049974090170624417752083431297576338548015787752535188523880852097525\
                                 63183032499671104356170739745011368293090619519590507334669961337861355\
                                 01688581"
#define KAZ_DS_SP_ORDERR        "14272684579936688348168733198222726360593450834975965161178920556932937\
                                 99664203253239731174839494234586883774385980429009019101571981529564489\
                                 47813901092072908919083052892315333580375982080000000000000000000000000\
                                 000000000000"
#define KAZ_DS_SP_FAC_ORDERR    18

#define KAZ_DS_ALPHABYTES       33
#define KAZ_DS_VBYTES           123
#define KAZ_DS_S1BYTES          34
#define KAZ_DS_S2BYTES          123
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
