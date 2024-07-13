#include "atf.h"

/* Expend a compressed atf to a N^3 array */
static void decompressATF(uint64_t *datf_out, const uint64_t *atf_in)
{
  int index=0;
  int i,j,k;
  for(i=0; i<N-2; i++)
    for(j=i+1; j<N-1; j++)
      for(k=j+1; k<N; k++)
	datf_out[i*(N*N)+j*N+k]=atf_in[index++];
}

/* Compress an expended atf */
static void compressATF(uint64_t *catf_out, const uint64_t *atf_in)
{
  int index=0;
  int i,j,k;
  for(i=0; i<N-2; i++)
    for(j=i+1; j<N-1; j++)
      for(k=j+1; k<N; k++)
	catf_out[index++]=reductionStrict(atf_in[i*(N*N)+j*N+k]);
}


/* Expend a compressed atf multiple time to multiple intertwined N^3 array */
static void compressATFS(uint64_t *catf_out, const uint64_t *atf_in, const int n_atf)
{
  int index=0;
  int i,j,k,r;

  for(r=0; r<n_atf; r++)
    for(i=0; i<N-2; i++)
      for(j=i+1; j<N-1; j++)
	for(k=j+1; k<N; k++)
	  catf_out[index++]=reductionStrict(atf_in[(i*(N*N)+j*N+k)*n_atf+r]);
}


/* Compress in a consecutive form multiple intertwined expended ATFE */
static void decompressATFS(uint64_t *datf_out, const uint64_t *atf_in, const int n_atf)
{
  int index=0;
  int i,j,k,r;

  for(i=0; i<N-2; i++)
    for(j=i+1; j<N-1; j++)
      for(k=j+1; k<N; k++)
	{
	  for(r=0; r<n_atf; r++)
	    datf_out[(i*(N*N)+j*N+k)*n_atf+r]=atf_in[index];
	  index++;
	}
}

/* Acting of column matrix on an atf */
static void actingOnATFwColumn(uint64_t *atf, uint64_t* col, const int j)
{
  /*  ORDER k,l,i*/

  /* i=j  */
 int i,k,l;

  /*k<l<j*/
  for(k=0; k<j-1; k++)
    for(l=k+1; l<j; l++)
      atf[k*(N*N)+l*N+j]=multiplicationModuloP(atf[k*(N*N)+l*N+j],col[j]);

  /*k<j<l*/
  for(k=0; k<j; k++)
    for(l=j+1; l<N; l++)
      atf[k*(N*N)+j*N+l]=multiplicationModuloP(atf[k*(N*N)+j*N+l],col[j]);

  /*j<k<l*/
  for(k=j+1; k<N-1; k++)
    for(l=k+1; l<N; l++)
      atf[j*(N*N)+k*N+l]=multiplicationModuloP(atf[j*(N*N)+k*N+l],col[j]);

  /*k<l<i<j*/
  for(k=0;k<j-2;k++)
    for(l=k+1;l<j-1;l++)
      for(i=l+1;i<j;i++)
	atf[k*(N*N)+l*N+j]+=multiplicationModuloP(col[i],atf[k*(N*N)+l*N+i]);

  /*k<i<l<j*/
  for(k=0;k<j-2;k++)
    for(l=k+2;l<j;l++)
      for(i=k+1;i<l;i++)
	atf[k*(N*N)+l*N+j]+=multiplicationModuloP((PRIME-col[i]),atf[k*(N*N)+i*N+l]);

  /*k<i<j<l*/
  for(k=0;k<j-1;k++)
    for(l=j+1;l<N;l++)
      for(i=k+1;i<j;i++)
	atf[k*(N*N)+j*N+l]+=multiplicationModuloP(col[i],atf[k*(N*N)+i*N+l]);

  /*i<k<l<j*/
  for(k=1;k<j-1;k++)
    for(l=k+1;l<j;l++)
      for(i=0;i<k;i++)
	atf[k*(N*N)+l*N+j]+=multiplicationModuloP(col[i],atf[i*(N*N)+k*N+l]);

  /*i<k<j<l*/
  for(k=1;k<j;k++)
    for(l=j+1;l<N;l++)
      for(i=0;i<k;i++)
	atf[k*(N*N)+j*N+l]+=multiplicationModuloP((PRIME-col[i]),atf[i*(N*N)+k*N+l]);

  /*i<j<k<l*/
  for(k=j+1;k<N-1;k++)
    for(l=k+1;l<N;l++)
      for(i=0;i<j;i++)
	atf[j*(N*N)+k*N+l]+=multiplicationModuloP(col[i],atf[i*(N*N)+k*N+l]);

  /*k<l<j<i*/
  for(k=0;k<j-1;k++)
    for(l=k+1;l<j;l++)
      for(i=j+1;i<N;i++)
	atf[k*(N*N)+l*N+j]+=multiplicationModuloP(col[i],atf[k*(N*N)+l*N+i]);

  /*k<j<l<i*/
  for(k=0;k<j;k++)
    for(l=j+1;l<N-1;l++)
      for(i=l+1;i<N;i++)
	atf[k*(N*N)+j*N+l]+=multiplicationModuloP((PRIME-col[i]),atf[k*(N*N)+l*N+i]);

  /*k<j<i<l*/
  for(k=0;k<j;k++)
    for(l=j+2;l<N;l++)
      for(i=j+1;i<l;i++)
	atf[k*(N*N)+j*N+l]+=multiplicationModuloP(col[i],atf[k*(N*N)+i*N+l]);

  /*j<k<l<i*/
  for(k=j+1;k<N-2;k++)
    for(l=k+1;l<N-1;l++)
      for(i=l+1;i<N;i++)
	atf[j*(N*N)+k*N+l]+=multiplicationModuloP(col[i],atf[k*(N*N)+l*N+i]);

  /*j<k<i<l*/
  for(k=j+1;k<N-2;k++)
    for(l=k+2;l<N;l++)
      for(i=k+1;i<l;i++)
	atf[j*(N*N)+k*N+l]+=multiplicationModuloP((PRIME-col[i]),atf[k*(N*N)+i*N+l]);

  /*j<i<k<l*/
  for(k=j+2;k<N-1;k++)
    for(l=k+1;l<N;l++)
      for(i=j+1;i<k;i++)
	atf[j*(N*N)+k*N+l]+=multiplicationModuloP(col[i],atf[i*(N*N)+k*N+l]);

  /*RED*/
  for(k=0; k<j-1; k++)
    for(l=k+1; l<j; l++)
      atf[k*(N*N)+l*N+j]=reductionModuloP(atf[k*(N*N)+l*N+j]);

  for(k=0; k<j; k++)
    for(l=j+1; l<N; l++)
      atf[k*(N*N)+j*N+l]=reductionModuloP(atf[k*(N*N)+j*N+l]);

  for(k=j+1; k<N-1; k++)
    for(l=k+1; l<N; l++)
      atf[j*(N*N)+k*N+l]=reductionModuloP(atf[j*(N*N)+k*N+l]);
}


/* Acting of column matrix on multipled intertwined atf */
static void actingOnATFSwColumn(uint64_t *atfs, const uint64_t* col, const int j, const int n_atf)
{
  /*  ORDER k,l,i */

  /* i=j */
  int i,k,l,r;

  /*k<l<j*/
  for(k=0; k<j-1; k++)
    for(l=k+1; l<j; l++)
      for(r=0;r<n_atf;r++)
	atfs[(k*(N*N)+l*N+j)*n_atf+r]=multiplicationModuloP(atfs[(k*(N*N)+l*N+j)*n_atf+r],col[j*n_atf+r]);

  /*k<j<l*/
  for(k=0; k<j; k++)
    for(l=j+1; l<N; l++)
      for(r=0;r<n_atf;r++)
      atfs[(k*(N*N)+j*N+l)*n_atf+r]=multiplicationModuloP(atfs[(k*(N*N)+j*N+l)*n_atf+r],col[j*n_atf+r]);

  /*j<k<l*/
  for(k=j+1; k<N-1; k++)
    for(l=k+1; l<N; l++)
      for(r=0;r<n_atf;r++)
      atfs[(j*(N*N)+k*N+l)*n_atf+r]=multiplicationModuloP(atfs[(j*(N*N)+k*N+l)*n_atf+r],col[j*n_atf+r]);

  /*k<l<i<j*/
  for(k=0;k<j-2;k++)
    for(l=k+1;l<j-1;l++)
      for(i=l+1;i<j;i++)
	for(r=0;r<n_atf;r++)
	atfs[(k*(N*N)+l*N+j)*n_atf+r]+=multiplicationModuloP(col[i*n_atf+r],atfs[(k*(N*N)+l*N+i)*n_atf+r]);

  /*k<i<l<j*/
  for(k=0;k<j-2;k++)
    for(l=k+2;l<j;l++)
      for(i=k+1;i<l;i++)
	for(r=0;r<n_atf;r++)
	atfs[(k*(N*N)+l*N+j)*n_atf+r]+=multiplicationModuloP((PRIME-col[i*n_atf+r]),atfs[(k*(N*N)+i*N+l)*n_atf+r]);

  /*k<i<j<l*/
  for(k=0;k<j-1;k++)
    for(l=j+1;l<N;l++)
      for(i=k+1;i<j;i++)
	for(r=0;r<n_atf;r++)
	atfs[(k*(N*N)+j*N+l)*n_atf+r]+=multiplicationModuloP(col[i*n_atf+r],atfs[(k*(N*N)+i*N+l)*n_atf+r]);

  /*i<k<l<j*/
  for(k=1;k<j-1;k++)
    for(l=k+1;l<j;l++)
      for(i=0;i<k;i++)
	for(r=0;r<n_atf;r++)
	atfs[(k*(N*N)+l*N+j)*n_atf+r]+=multiplicationModuloP(col[i*n_atf+r],atfs[(i*(N*N)+k*N+l)*n_atf+r]);

  /*i<k<j<l*/
  for(k=1;k<j;k++)
    for(l=j+1;l<N;l++)
      for(i=0;i<k;i++)
	for(r=0;r<n_atf;r++)
	atfs[(k*(N*N)+j*N+l)*n_atf+r]+=multiplicationModuloP((PRIME-col[i*n_atf+r]),atfs[(i*(N*N)+k*N+l)*n_atf+r]);

  /*i<j<k<l*/
  for(k=j+1;k<N-1;k++)
    for(l=k+1;l<N;l++)
      for(i=0;i<j;i++)
	for(r=0;r<n_atf;r++)
	atfs[(j*(N*N)+k*N+l)*n_atf+r]+=multiplicationModuloP(col[i*n_atf+r],atfs[(i*(N*N)+k*N+l)*n_atf+r]);

  /*k<l<j<i*/
  for(k=0;k<j-1;k++)
    for(l=k+1;l<j;l++)
      for(i=j+1;i<N;i++)
	for(r=0;r<n_atf;r++)
	atfs[(k*(N*N)+l*N+j)*n_atf+r]+=multiplicationModuloP(col[i*n_atf+r],atfs[(k*(N*N)+l*N+i)*n_atf+r]);

  /*k<j<l<i*/
  for(k=0;k<j;k++)
    for(l=j+1;l<N-1;l++)
      for(i=l+1;i<N;i++)
	for(r=0;r<n_atf;r++)
	atfs[(k*(N*N)+j*N+l)*n_atf+r]+=multiplicationModuloP((PRIME-col[i*n_atf+r]),atfs[(k*(N*N)+l*N+i)*n_atf+r]);

  /*k<j<i<l*/
  for(k=0;k<j;k++)
    for(l=j+2;l<N;l++)
      for(i=j+1;i<l;i++)
	for(r=0;r<n_atf;r++)
	atfs[(k*(N*N)+j*N+l)*n_atf+r]+=multiplicationModuloP(col[i*n_atf+r],atfs[(k*(N*N)+i*N+l)*n_atf+r]);

  /*j<k<l<i*/
  for(k=j+1;k<N-2;k++)
    for(l=k+1;l<N-1;l++)
      for(i=l+1;i<N;i++)
	for(r=0;r<n_atf;r++)
	atfs[(j*(N*N)+k*N+l)*n_atf+r]+=multiplicationModuloP(col[i*n_atf+r],atfs[(k*(N*N)+l*N+i)*n_atf+r]);

  /*j<k<i<l*/
  for(k=j+1;k<N-2;k++)
    for(l=k+2;l<N;l++)
      for(i=k+1;i<l;i++)
	for(r=0;r<n_atf;r++)
	atfs[(j*(N*N)+k*N+l)*n_atf+r]+=multiplicationModuloP((PRIME-col[i*n_atf+r]),atfs[(k*(N*N)+i*N+l)*n_atf+r]);

  /*j<i<k<l*/
  for(k=j+2;k<N-1;k++)
    for(l=k+1;l<N;l++)
      for(i=j+1;i<k;i++)
	for(r=0;r<n_atf;r++)
	atfs[(j*(N*N)+k*N+l)*n_atf+r]+=multiplicationModuloP(col[i*n_atf+r],atfs[(i*(N*N)+k*N+l)*n_atf+r]);

  /*RED*/
  for(k=0; k<j-1; k++)
    for(l=k+1; l<j; l++)
      for(r=0;r<n_atf;r++)
      atfs[(k*(N*N)+l*N+j)*n_atf+r]=reductionModuloP(atfs[(k*(N*N)+l*N+j)*n_atf+r]);

  for(k=0; k<j; k++)
    for(l=j+1; l<N; l++)
      for(r=0;r<n_atf;r++)
      atfs[(k*(N*N)+j*N+l)*n_atf+r]=reductionModuloP(atfs[(k*(N*N)+j*N+l)*n_atf+r]);

  for(k=j+1; k<N-1; k++)
    for(l=k+1; l<N; l++)
      for(r=0;r<n_atf;r++)
      atfs[(j*(N*N)+k*N+l)*n_atf+r]=reductionModuloP(atfs[(j*(N*N)+k*N+l)*n_atf+r]);
}

/* Acting of column matrix on multipled consecutive compressed atf */
void actingOnATFS(uint64_t *atf_out, const uint64_t *atf_in, const uint64_t *columns, const int n_atf)
{
  uint64_t* atfs = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*N*n_atf, 64);
  uint64_t* cols = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*n_atf, 64);

  int i,j,r;
  for (i=0;i<N*N;i++)
    for (r=0;r<n_atf;r++)
      cols[i*n_atf+r]=columns[r*N*N+i];

  decompressATFS(atfs, atf_in, n_atf);
  for(j=0;j<N;j++)
    actingOnATFSwColumn(atfs, &(cols[j*N*n_atf]), j, n_atf);
  compressATFS(atf_out, atfs, n_atf);

  /* free */
  _mm_free(cols);
  _mm_free(atfs);
}


/* Inverting of N column matrix on a compressed atf */
void invertingOnATF(uint64_t *atf_out, const uint64_t *atf_in, const uint64_t *columns)
{
  uint64_t* atf = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*N, 64);
  uint64_t diagonal[N];
  uint64_t column[N];

  int i,j;
  decompressATF(atf, atf_in);
  for(i=0;i<N;i++)
    diagonal[i]=columns[i*(N+1)];
  setInversionModuloP(diagonal);
  for(j=N-1;j>=0;j--){
    for(i=0;i<N;i++)
      if (i!=j)
	column[i]=reductionModuloP(multiplicationModuloP((PRIME-diagonal[j]),columns[j*N+i]));
    column[j]=diagonal[j];
    actingOnATFwColumn(atf, column, j);
  }
  compressATF(atf_out, atf);

  /* free */
  _mm_free(atf);
}

/* Acting of K matrix on K consecutive compressed atf independently */
void actingOnATFSwTensor(uint64_t *atf0, uint64_t *atf1, uint64_t* mat)
{

  uint64_t* tensorA = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*N*K, 64);
  uint64_t* tensorB = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*N*K, 64);
  uint64_t* tran    = (uint64_t*)_mm_malloc(sizeof(uint64_t) * N*N*K, 64);
  int index=0;

  int i,j,k,r,l;

  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      for(r=0;r<K;r++)
	tran[(i*N+j)*K+r]=mat[r*N*N+j*N+i];

  for(r=0;r<K;r++)
    for(i=0; i<N-2; i++)
      for(j=i+1; j<N-1; j++)
	for(k=j+1; k<N; k++)
	  {
	    tensorA[(i*(N*N)+j*N+k)*K+r]=atf0[index];
	    tensorA[(j*(N*N)+i*N+k)*K+r]=PRIME-atf0[index];
	    tensorA[(k*(N*N)+j*N+i)*K+r]=PRIME-atf0[index];
	    tensorA[(i*(N*N)+k*N+j)*K+r]=PRIME-atf0[index];
	    tensorA[(j*(N*N)+k*N+i)*K+r]=atf0[index];
	    tensorA[(k*(N*N)+i*N+j)*K+r]=atf0[index++];
	  }

  for(i=0;i<N;i++)
    for(j=0; j<N; j++)
      for(r=0;r<K;r++)
	{
	  tensorA[(i*(N*N)+i*N+j)*K+r]=0;
	  tensorA[(i*(N*N)+j*N+i)*K+r]=0;
	  tensorA[(j*(N*N)+i*N+i)*K+r]=0;
	}

  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      for(k=0;k<N;k++)
	{
	  for(r=0;r<K;r++)
	    tensorB[(i*(N*N)+j*N+k)*K+r]=0;
	  for(l=0;l<N;l++)
	    for(r=0;r<K;r++)
	      tensorB[(i*(N*N)+j*N+k)*K+r]+=multiplicationModuloP(tran[(j*N+l)*K+r],tensorA[(i*(N*N)+l*N+k)*K+r]);
	  for(r=0;r<K;r++)
	    tensorB[(i*(N*N)+j*N+k)*K+r]=reductionModuloP(tensorB[(i*(N*N)+j*N+k)*K+r]);
	}

  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      for(k=j+1;k<N;k++)
	{
	  for(r=0;r<K;r++)
	    tensorA[(i*(N*N)+j*N+k)*K+r]=0;
	  for(l=0;l<N;l++)
	    for(r=0;r<K;r++)
	      tensorA[(i*(N*N)+j*N+k)*K+r]+=multiplicationModuloP(tran[(k*N+l)*K+r],tensorB[(i*(N*N)+j*N+l)*K+r]);
	  for(r=0;r<K;r++)
	    tensorA[(i*(N*N)+j*N+k)*K+r]=reductionModuloP(tensorA[(i*(N*N)+j*N+k)*K+r]);
	}

  for(i=0;i<N-2;i++)
    for(j=i+1;j<N-1;j++)
      for(k=j+1;k<N;k++)
	{
	  for(r=0;r<K;r++)
	    tensorB[(i*(N*N)+j*N+k)*K+r]=0;
	  for(l=0;l<N;l++)
	    for(r=0;r<K;r++)
	      tensorB[(i*(N*N)+j*N+k)*K+r]+=multiplicationModuloP(tran[(i*N+l)*K+r],tensorA[(l*(N*N)+j*N+k)*K+r]);
	  for(r=0;r<K;r++)
	    tensorB[(i*(N*N)+j*N+k)*K+r]=reductionModuloP(tensorB[(i*(N*N)+j*N+k)*K+r]);
	}


  index=0;
  for(r=0;r<K;r++)
    for(i=0;i<N-2;i++)
      for(j=i+1;j<N-1;j++)
	for(k=j+1;k<N;k++)
	  atf1[index++]=reductionStrict(tensorB[(i*(N*N)+j*N+k)*K+r]);

  /* free */
  _mm_free(tensorA);
  _mm_free(tensorB);
  _mm_free(tran);

}

