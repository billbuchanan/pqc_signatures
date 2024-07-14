#include "matrix.h"

/* Multiplication of a matrix by a column matrix with the columns with first elements equal to 0 */
static void columnMul(uint64_t *mat, const uint64_t *column, const int j, const int s)
{
  int i,k;

  for(i=0;i<N;i++)
    if (i!=j)
      for(k=s;k<N;k++)
	mat[i*N+k]=reductionModuloP(mat[i*N+k]+multiplicationModuloP(mat[j*N+k],column[i]));
  for(k=s;k<N;k++)
    mat[j*N+k]=reductionModuloP(multiplicationModuloP(mat[j*N+k],column[j]));
}

/* Creation of a matrix representing the product of n columns matrix by n columns matrix */
void columnsMatrix(uint64_t *mat, const uint64_t *colsA, const uint64_t *colsB)
{
  int i,j;

  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      mat[i*N+j]=colsA[j*N+i];
  for(j=N-1;j>=0;j--)
    columnMul(mat, &(colsA[j*N]), j, j+1);
  for(j=N-1;j>=0;j--)
    columnMul(mat, &(colsB[j*N]), j, 0);
}
