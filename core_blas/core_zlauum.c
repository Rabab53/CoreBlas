/**
 *
 * @file
 *
 *  COREBLAS is a software package provided by:
 *  University of Tennessee, US,
 *  University of Manchester, UK.
 *
 * @precisions normal z -> c d s
 *
 **/

#include <coreblas.h>
#include "coreblas_types.h"
#include "core_lapack.h"

/***************************************************************************//**
 *
 * @ingroup core_lauum
 *
 *  Computes the product U * U^H or L^H * L, where the triangular
 *  factor U or L is stored in the upper or lower triangular part of
 *  the array A.
 *
 *  If uplo = 'U' or 'u' then the upper triangle of the result is stored,
 *  overwriting the factor U in A.
 *  If uplo = 'L' or 'l' then the lower triangle of the result is stored,
 *  overwriting the factor L in A.

 *
 *******************************************************************************
 *
 * @param[in] uplo
 *          = CoreBlasUpper: Upper triangle of A is stored;
 *          = CoreBlasLower: Lower triangle of A is stored.
 *
 *
 * @param[in] n
 *          The order of the matrix A. n >= 0.
 *
 * @param[in,out] A
 *          On entry, the triangular factor U or L.
 *          On exit, if uplo = 'U', the upper triangle of A is
 *          overwritten with the upper triangle of the product U * U^H;
 *          if uplo = 'L', the lower triangle of A is overwritten with
 *          the lower triangle of the product L^H * L.

 *
 * @param[in] lda
 *          The leading dimension of the array A. lda >= max(1,n).
 *
 * @param[out] info
 *          - 0 on successful exit
 *          - < 0 if -i, the i-th argument had an illegal value
 *
 ******************************************************************************/
__attribute__((weak))
int coreblas_zlauum(coreblas_enum_t uplo,
                int n,
                coreblas_complex64_t *A, int lda)
{
    #ifdef COREBLAS_USE_64BIT_BLAS
        return LAPACKE_zlauum64_(LAPACK_COL_MAJOR,
                        lapack_const(uplo), n, A, lda);
    #else
        return LAPACKE_zlauum_work(LAPACK_COL_MAJOR,
                        lapack_const(uplo), n, A, lda);
    #endif 

}