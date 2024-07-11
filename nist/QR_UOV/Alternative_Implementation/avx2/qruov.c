#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "qruov.h"
#include <string.h>

/* ---------------------------------------------------------
   local definition
   --------------------------------------------------------- */

#define ceil_log_2_q QRUOV_ceil_log_2_q
#define v            QRUOV_v
#define m            QRUOV_m
#define L            QRUOV_L
#define fc           QRUOV_fc
#define fe           QRUOV_fe
#define q            QRUOV_q
#define n            QRUOV_n
#define N            QRUOV_N
#define V            QRUOV_V
#define M            QRUOV_M

/* ---------------------------------------------------------
   KeyGen
   --------------------------------------------------------- */

static void SAMPLE_MATRIX_VxM (Fql_RANDOM_CTX ctx, MATRIX_VxM C) {
  for(int j=0;j<V;j++){
    for(int k=0;k<M;k++){
      C[j][k] = Fql_random(ctx) ;
    }
  }
}

static void SAMPLE_TRANSPOSE_MATRIX_MxV (Fql_RANDOM_CTX ctx, MATRIX_MxV C) {
  for(int j=0;j<V;j++){
    for(int k=0;k<M;k++){
      C[k][j] = Fql_random(ctx) ;
    }
  }
}

static void SAMPLE_SYMMETRIC_MATRIX_VxV (Fql_RANDOM_CTX ctx, MATRIX_VxV C) {
  for(int j=0; j<V; j++){
    for(int k=0; k<V; k++){
      if(k<j){
        C[j][k] = C[k][j] ;
      }else{
        C[j][k] = Fql_random(ctx) ;
      }
    }
  }
}

static void SAMPLE_Sd(const QRUOV_SEED sk_seed, MATRIX_VxM Sd, MATRIX_MxV SdT){
  Fql_RANDOM_CTX ctx ;
  Fql_srandom(sk_seed, ctx) ;
  SAMPLE_MATRIX_VxM(ctx, Sd) ;
  MATRIX_TRANSPOSE_VxM(Sd, SdT) ;
  Fql_random_final(ctx) ;
}

#define SAMPLE_Pi1(ctx, Pi1)   SAMPLE_SYMMETRIC_MATRIX_VxV(ctx, Pi1)
#define SAMPLE_Pi2(ctx, Pi2)   SAMPLE_MATRIX_VxM(ctx, Pi2)

// Pi3 = (Pi2^T - Sd^T * Pi1) * Sd + Sd^T * Pi2
static void SAMPLE_Pi3(
  MATRIX_VxM Sd,   // input
  MATRIX_MxV SdT,  //
  MATRIX_VxV Pi1,  // ...
  MATRIX_VxM Pi2,  //
  MATRIX_MxV Pi2T, // input
  MATRIX_MxM Pi3   // output
){
  MATRIX_MxV TMP ;

  MATRIX_MUL_MxV_VxV(SdT,Pi1,TMP) ;
  MATRIX_SUB_MxV(Pi2T,TMP,TMP) ;
  MATRIX_MUL_MxV_VxM(TMP,Sd,Pi3) ;
  MATRIX_MUL_ADD_MxV_VxM(SdT,Pi2,Pi3) ;

  OPENSSL_cleanse(TMP, sizeof(TMP)) ;
}

static void SAMPLE_P1P2(const QRUOV_SEED pk_seed, QRUOV_P1 P1, QRUOV_P2 P2, QRUOV_P2T P2T){
  Fql_RANDOM_CTX ctx ;
  Fql_srandom(pk_seed, ctx) ;
  for(int i=0;i<m;i++){
    SAMPLE_Pi1(ctx, P1[i]) ;
  }
  for(int i=0;i<m;i++){
    SAMPLE_Pi2(ctx, P2[i]) ;
    MATRIX_TRANSPOSE_VxM(P2[i], P2T[i]) ;
  }
  Fql_random_final(ctx) ;
}

static void SAMPLE_P3(
  MATRIX_VxM Sd,  // input
  MATRIX_MxV SdT, //
  QRUOV_P1   P1,  // ...
  QRUOV_P2   P2,  //
  QRUOV_P2T  P2T, // input
  QRUOV_P3   P3   // output
){
  int i ;
#pragma omp parallel for private(i) shared(Sd, SdT, P1, P2, P2T, P3)
  for(i=0;i<m;i++){
    SAMPLE_Pi3(Sd, SdT, P1[i], P2[i], P2T[i], P3[i]) ;
  }
}


inline static void * aligned_calloc(size_t alignment, size_t size){
  void * p = aligned_alloc(alignment, size) ;
  if(p) memset(p,0,size) ;
  return p ;
}

void QRUOV_KeyGen (
  const QRUOV_SEED sk_seed, // input
  const QRUOV_SEED pk_seed, // input
  QRUOV_P3         P3       // output
){
  // Huge array
  VECTOR_M   * Sd   = (VECTOR_M *)   aligned_calloc(sizeof(__m256i), sizeof(MATRIX_VxM)) ;
  VECTOR_V   * SdT  = (VECTOR_V *)   aligned_calloc(sizeof(__m256i), sizeof(MATRIX_MxV)) ;

  MATRIX_VxV * P1   = (MATRIX_VxV *) aligned_calloc(sizeof(__m256i),sizeof(QRUOV_P1)) ;
  MATRIX_VxM * P2   = (MATRIX_VxM *) aligned_calloc(sizeof(__m256i),sizeof(QRUOV_P2)) ;
  MATRIX_MxV * P2T  = (MATRIX_MxV *) aligned_calloc(sizeof(__m256i),sizeof(QRUOV_P2T)) ;

  if ( (Sd==NULL) || (SdT==NULL) || (P1==NULL) || (P2==NULL) || (P2T==NULL) ) {
    ERROR_ABORT("malloc fail") ;
  }

  SAMPLE_Sd(sk_seed, Sd, SdT) ;
  SAMPLE_P1P2(pk_seed, P1, P2, P2T) ;
  SAMPLE_P3(Sd, SdT, P1, P2, P2T, P3) ;

  OPENSSL_cleanse(Sd, sizeof(MATRIX_VxM)) ;
  OPENSSL_cleanse(SdT, sizeof(MATRIX_MxV)) ;
  free(Sd) ; free(SdT) ; free(P1) ; free(P2) ; free(P2T) ;
  return ;
}

/* ---------------------------------------------------------
   Sign
   --------------------------------------------------------- */

// Fi2  = - Pi1 * Sd  + Pi2  ;
// Fi2T = - SdT * Pi1 + Pi2T ;

static void SAMPLE_Fi2T(
  MATRIX_MxV SdT,  // input
  MATRIX_VxV Pi1,  //
  MATRIX_MxV Pi2T, // ... 

  MATRIX_MxV Fi2T  // output
){
  MATRIX_MUL_MxV_VxV(SdT,Pi1,Fi2T) ;
  MATRIX_SUB_MxV(Pi2T,Fi2T,Fi2T) ;
}

static void SAMPLE_F2T(
  MATRIX_MxV   SdT,  // input
  MATRIX_VxV * P1,   //
  MATRIX_MxV * P2T,  // ... 

  MATRIX_MxV * F2T   // output
){
  int i ;
#pragma omp parallel for private(i) shared(SdT, P1, P2T, F2T)
  for(i=0; i<m; i++){
    SAMPLE_Fi2T(SdT, P1[i], P2T[i], F2T[i]) ;
  }
}
// ==============================================
// linear algebra
// ==============================================

TYPEDEF_STRUCT (ROW,
  Fq * col  ;
  int  original_row_id ;
) ;

TYPEDEF_STRUCT (ECHELON_FORM,
  ROW_s   row   [m] ;
  ROW_s * eqn   [m] ;
  int rank          ;
  int index     [m] ;
) ;

static void echelon_form_init(Fq mat[m][m], ECHELON_FORM echelon_form) {
  for(int i=0;i<m;i++){
    echelon_form->row[i].col             = mat[i] ;
    echelon_form->row[i].original_row_id = i ;
    echelon_form->eqn[i]                 = echelon_form->row + i ;
  }
  echelon_form->rank = 0 ;
  memset(echelon_form->index,-1,sizeof(echelon_form->index));
}

static void row_swap(ROW_s * eqn[m], int i, int j) {
  ROW_s * tmp  = eqn[i] ;
  eqn[i] = eqn[j] ;
  eqn[j] = tmp ;
}

#define EQN(I,J) (eqn[I]->col[J])

static void LU_decompose(
  Fq A[m][m],                    // input (will be destroyed)
  ECHELON_FORM echelon_form      // output
) {
  echelon_form_init(A, echelon_form) ;
  int i,j,k ;
  ROW_s ** eqn = echelon_form->eqn ;

  int c = -1 ;
  for(i=0; i<m; i++) {
    c++ ; if(c >= m) return ;
    j = i ;
    while(EQN(j,c)==0){
      j++ ;
      if(j >= m) {
        c++ ; if(c >= m) return ;
        j = i ;
      }
    }

    row_swap(eqn,i,j) ;
    echelon_form->index[echelon_form->rank++] = c ;

    Fq pivot = EQN(i,c) ;
    Fq inv   = Fq_inv(pivot) ;

    EQN(i,i)  = pivot ;
    for(k = c+1; k<m; k++) EQN(i,k) = Fq_mul(inv, EQN(i,k)) ;

    for(j=i+1; j<m; j++) {
      Fq mul = EQN(j,c) ;
      EQN(j,i) = mul ;

      for(k = c+1; k<m; k++){
        EQN(j,k) = Fq_sub(EQN(j,k), Fq_mul(mul, EQN(i,k))) ;
      }
    }
  }

  return ;
}

static void Fq_mxm_identity(Fq A[m][m]){
  memset(A,0,sizeof(Fq)*m*m) ;
  for(int i=0;i<m;i++) A[i][i] = 1 ;
}

static void L_inverse(ECHELON_FORM echelon_form, Fq R[m][m]){
  ROW_s ** eqn  = echelon_form->eqn  ;
  int      rank = echelon_form->rank ;

  Fq_mxm_identity(R) ;

  for(int i=0;i<rank;i++){
    Fq pivot = EQN(i,i) ;
    Fq inv   = Fq_inv(pivot) ;
    for(int k=0;k<=i;k++) R[i][k] = Fq_mul(R[i][k],inv) ;
    for(int j=i+1;j<m;j++){
      for(int k=0;k<=i;k++){
        R[j][k]  = Fq_sub(R[j][k], Fq_mul(EQN(j,i),R[i][k])) ;
      }
    }
  }
}

static int consistent (
  ECHELON_FORM echelon_form,   // input
  Fq b[m],
  int * cacheR,                // output
  Fq R[m][m]
) {
  ROW_s ** eqn  = echelon_form->eqn  ;
  int      rank = echelon_form->rank ;
  if(rank==m) return 1 ;

  if(*cacheR == 0){
    L_inverse(echelon_form, R) ;
    *cacheR = 1;
  }

  for(int i=rank; i<m; i++){
    int     k ;
    uint64_t t = 0 ;
    for(int j=0; j<rank; j++){
      k = eqn[j]->original_row_id ;
      t += ((uint64_t)R[i][j]) * ((uint64_t)b[k]) ;
    }
    k = eqn[i]->original_row_id ;
    t += ((uint64_t)R[i][i]) * ((uint64_t)b[k]) ;
    t %= QRUOV_q ;
    if(t) return 0 ;
  }

  return 1 ;
}

static void sample_a_solution(
  Fql_RANDOM_CTX ctx,                // input
  ECHELON_FORM   echelon_form,       // input
  Fq             b    [m],           // input
  Fq             x    [m],           // output
  Fq             b2   [m]            // output
) {
  int      rank  = echelon_form->rank ;
  int   *  index = echelon_form->index ;
  ROW_s ** eqn   = echelon_form->eqn  ;
  int i,j,k,o ;

  for(i=0; i<rank; i++){
    uint64_t t = 0 ;
    for(j=0;j<i;j++){
      t += ((uint64_t)EQN(i,j)) * ((uint64_t)b2[j]) ;
    }
    Fq tmp = (Fq)(t % QRUOV_q) ;
    k = eqn[i]->original_row_id ;
    tmp = Fq_sub(b[k],tmp) ;
    b2[i] = Fq_mul(tmp,Fq_inv(EQN(i,i))) ;
  }

  i = m-1 ;
  for(j = rank-1; j>=0; j--){
    k = index[j] ;
    for( ; i > k; i--) x[i] = Fq_random(ctx) ;
    // i == k
    uint64_t t = 0 ;
    for(k++; k < m; k++){
      t += ((uint64_t)EQN(j,k)) * ((uint64_t)(x[k])) ;
    }
    x[i] = Fq_sub(b2[j], (Fq)(t % QRUOV_q) ) ;
    i-- ;
  }
  for( ; i >= 0; i--) x[i] = Fq_random(ctx) ;
  return ;
}

static Fql * pack_0 (Fq oil_u[m], Fql oil[M]) {
  for(int i=0; i<M; i++){
    for(int j=0; j<L; j++){
      oil[i].c[j] = oil_u[i*L+j] ;
    }
  }
  return oil ;
}

void QRUOV_Sign (
  const QRUOV_SEED sk_seed,      // input
  const QRUOV_SEED pk_seed,      // input

  const QRUOV_SEED vineger_seed, // input
  const QRUOV_SEED r_seed,       // input

  const uint8_t    Msg[],        // input
  const size_t     Msg_len,      // input

  QRUOV_SIGNATURE  sig           // output
) {
  int i,j,k,o ;

  Fql_RANDOM_CTX ctx ;

  ECHELON_FORM echelon_form ;
  Fql vineger [V] ;
  Fql oil     [M] ; //

  Fq  oil_u   [m] ; // unpacked oil.
  Fq  b       [m] ; //
  Fq  b2      [m] ; //
  Fq  c       [m] ; //

  /* ----------------------------------
     Huge array -> malloc
     ---------------------------------- */

  VECTOR_M   * Sd   = (VECTOR_M   *)  aligned_calloc(sizeof(__m256i), sizeof(MATRIX_VxM)) ;
  VECTOR_V   * SdT  = (VECTOR_V   *)  aligned_calloc(sizeof(__m256i), sizeof(MATRIX_MxV)) ;

  MATRIX_VxV * P1   = (MATRIX_VxV *)  aligned_calloc(sizeof(__m256i), sizeof(QRUOV_P1)) ;
  MATRIX_VxM * P2   = (MATRIX_VxM *)  aligned_calloc(sizeof(__m256i), sizeof(QRUOV_P2)) ;

  MATRIX_MxV * P2T  = (MATRIX_MxV *)  aligned_calloc(sizeof(__m256i), sizeof(QRUOV_P2T)) ;
  MATRIX_VxV * F1   = P1 ;
  MATRIX_MxV * F2T  = (MATRIX_MxV *)  aligned_calloc(sizeof(__m256i), sizeof(QRUOV_P2T)) ;

  Fq (* eqn)[m]     = (Fq(*)[m])      aligned_calloc(sizeof(__m256i), sizeof(Fq)*m*(m)) ;

  Fq (* R)[m]       = (Fq(*)[m])      aligned_calloc(sizeof(__m256i), sizeof(Fq)*m*m) ;
  int cacheR        = 0 ;

  if ( (Sd==NULL)|| (SdT==NULL)||(P1==NULL)|| (P2==NULL)|| (P2T==NULL)||(F2T==NULL)|| (eqn==NULL)||(R==NULL) ){
    ERROR_ABORT("malloc fail") ;
  }

  /* ----------------------------------
     
     ---------------------------------- */

  Fq msg [QRUOV_m] ;

  SAMPLE_Sd(sk_seed, Sd, SdT) ;
  SAMPLE_P1P2(pk_seed, P1, P2, P2T) ;
  SAMPLE_F2T(SdT, P1, P2T, F2T) ;

  Fql_srandom(vineger_seed, ctx) ;
  Fql_random_vector(ctx, V, vineger) ;

#pragma omp parallel for private(i,j,k) shared(vineger, F2T, eqn)
  for(i=0;i<m; i++){
    for(j=0; j<M; j++){
      Fql t = Fql_zero ;
      for(k=0; k<V; k++){
        t = Fql_add(t, Fql_mul(vineger[k],F2T[i][j][k])) ;
      }
      t = Fql_add(t,t) ;
      for(int l=0; l<L; l++){
        eqn[i][L*j+l] = t.c[QRUOV_perm(l)] ; // <- unpack_1(...) 
      }
    }
  }

  LU_decompose(eqn, echelon_form) ;

#pragma omp parallel for private(i,j,k) shared(vineger, F1, c)
  for(i=0;i<m; i++){
    Fql tmp [V] ;

    for(j=0; j<V; j++){
      Fql t = Fql_zero ;
      for(k=0; k<V; k++){
        t = Fql_add(t, Fql_mul(vineger[k],F1[i][j][k])) ;
      }
      tmp[j] = t ;
    }

    uint64_t c_i = 0 ;
    for(k=0; k<V; k++){
      c_i += (uint64_t)(Fql_mul(tmp[k],vineger[k]).c[QRUOV_perm(0)]); // <-- shrink
    }

    c[i] = (Fq)(c_i % QRUOV_q) ;
  }

  Fql_RANDOM_CTX msg_ctx   ; Fql_srandom_init(Msg, Msg_len, msg_ctx) ;
  Fql_RANDOM_CTX msg_ctx_2 ;
  MGF_CTX        r_ctx     ; MGF_init(r_seed, QRUOV_SEED_LEN, r_ctx) ;
  do{
    MGF_yield(r_ctx, sig->r, QRUOV_SALT_LEN) ;
    Fql_RANDOM_CTX_copy(msg_ctx, msg_ctx_2) ;
    Fql_srandom_update(sig->r, QRUOV_SALT_LEN, msg_ctx_2) ;
    for(i=0; i<m; i++) msg[i] = Fq_random(msg_ctx_2) ;
    for(i=0; i<m; i++) b[i] = Fq_sub(msg[i], c[i]) ;
    Fq_random_final(msg_ctx_2) ;
  }while(!consistent(echelon_form, b, &cacheR, R)) ;
  MGF_final(r_ctx) ;
  Fq_random_final(msg_ctx) ;

  sample_a_solution(ctx, echelon_form, b, oil_u, b2) ;
  Fql_random_final(ctx) ;

  pack_0(oil_u, oil) ;

  for(i=0;i<V;i++){
    Fql t = Fql_zero ;
    for(j=0;j<M;j++){
      t = Fql_add(t, Fql_mul(oil[j],SdT[j][i])) ;
    }
    sig->s[i] = Fql_sub(vineger[i], t) ;
  }
  for(i=V;i<N;i++){
    sig->s[i] = oil[i-V] ;
  }

  OPENSSL_cleanse(Sd, sizeof(MATRIX_VxM)) ;
  OPENSSL_cleanse(SdT, sizeof(MATRIX_MxV)) ;
  free(Sd); free(SdT); free(P1); free(P2); free(P2T); free(F2T); free(eqn); free(R);
  return ;
}

/* ---------------------------------------------------------
   Verify
   --------------------------------------------------------- */

int QRUOV_Verify(
  const QRUOV_SEED       pk_seed,     // input
  const QRUOV_P3         P3,          // input

  const uint8_t          Msg[],       // input
  const size_t           Msg_len,     // input

  const QRUOV_SIGNATURE  sig          // input
) {
  Fql_RANDOM_CTX msg_ctx ;
  Fql_srandom_init(Msg, Msg_len, msg_ctx) ;
  Fql_srandom_update(sig->r, QRUOV_SALT_LEN, msg_ctx) ;

  Fq msg [QRUOV_m] ;

  for(int i=0; i<m; i++) msg[i] = Fq_random(msg_ctx) ;

  MATRIX_VxV * P1   = (MATRIX_VxV *)  aligned_calloc(sizeof(__m256i), sizeof(QRUOV_P1)) ;
  MATRIX_VxM * P2   = (MATRIX_VxM *)  aligned_calloc(sizeof(__m256i), sizeof(QRUOV_P2)) ;
  MATRIX_MxV * P2T  = (MATRIX_MxV *)  aligned_calloc(sizeof(__m256i), sizeof(QRUOV_P2T)) ;

  if ( (P1==NULL) || (P2==NULL) || (P2T==NULL) ) {
    ERROR_ABORT("malloc fail") ;
  }


  const Fql * vineger = sig->s ;
  const Fql * oil     = sig->s + V ;
  Fql t ;
  int i,j,k ;

  SAMPLE_P1P2(pk_seed, P1, P2, P2T) ;

  int result [m] ;
#pragma omp parallel for private(i,j,k,t) shared(P1, P2T, P3, oil, vineger, msg, result)
  for(i=0; i<m; i++){
    Fql tmp_v [V] ;
    Fql tmp_o [M] ;
    for(j=0;j<V;j++){
      t = Fql_zero ;
      for(k=0;k<M;k++){
        t = Fql_add(t, Fql_mul(P2T[i][k][j],oil[k])) ; // <-
      }
      t = Fql_add(t,t) ;
      for(k=0;k<V;k++){
        t = Fql_add(t, Fql_mul(P1[i][j][k],vineger[k])) ;
      }
      tmp_v[j] = t ;
    }

    for(j=0;j<M;j++){
      t = Fql_zero ;
      for(k=0;k<M;k++){
        t = Fql_add(t, Fql_mul(P3[i][j][k],oil[k])) ;
      }
      tmp_o[j] = t ;
    }

    t = Fql_zero ;
    for(j=0;j<V;j++){
      t = Fql_add(t, Fql_mul(vineger[j],tmp_v[j])) ;
    }
    for(j=0;j<M;j++){
      t = Fql_add(t, Fql_mul(oil[j],tmp_o[j])) ;
    }
    if(msg[i] != t.c[QRUOV_perm(0)]){ // <-- shrink
      result[i] = 0 ;
    }else{
      result[i] = 1 ;
    }
  }
  free(P1) ; free(P2) ; free(P2T) ;
  for(i=0;i<m;i++){
    if(result[i] == 0) return 0 ;
  }
  return 1 ;
}

/* ---------------------------------------------------------
   memory I/O
   --------------------------------------------------------- */

void store_QRUOV_P3(
  const QRUOV_P3   P3,       // input
  uint8_t        * pool,     // output
  size_t         * pool_bits // input/output (current bit index)
){
  for(int i=0; i<m; i++){
    for(int j=0; j<M; j++){
      for(int k=0; k<M; k++){
        if(k<j){
          // do nothing
          // if(P3[i][j][k] != P3[i][k][j]){ printf("error\n"); };
	}else{
          store_Fql(P3[i][j][k], pool, pool_bits) ;
        }
      }
    }
  }
}

void restore_QRUOV_P3(
  const uint8_t * pool,      // input
  size_t        * pool_bits, // input/output (current bit index)
  QRUOV_P3        P3         // output
){
  for(int i=0; i<m; i++){
    for(int j=0; j<M; j++){
      for(int k=0; k<M; k++){
        if(k<j){
          P3[i][j][k] = P3[i][k][j] ;
        }else{
          P3[i][j][k] = restore_Fql(pool, pool_bits) ;
        }
      }
    }
  }
}
