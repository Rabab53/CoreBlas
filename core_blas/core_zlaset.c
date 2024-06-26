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
#include "coreblas_internal.h"
#include "core_lapack.h"

// for memset function
#include <string.h>

/***************************************************************************//**
 *
 * @ingroup core_laset
 *
 *  Sets the elements of the matrix A on the diagonal
 *  to beta and on the off-diagonals to alpha
 *
 *******************************************************************************
 *
 * @param[in] uplo
 *          Specifies which elements of the matrix are to be set
 *          - CoreBlasUpper: Upper part of A is set;
 *          - CoreBlasLower: Lower part of A is set;
 *          - CoreBlasUpperLower: ALL elements of A are set.
 *
 * @param[in] m
 *          The number of rows of the matrix A.  m >= 0.
 *
 * @param[in] n
 *         The number of columns of the matrix A.  n >= 0.
 *
 * @param[in] alpha
 *         The constant to which the off-diagonal elements are to be set.
 *
 * @param[in] beta
 *         The constant to which the diagonal elements are to be set.
 *
 * @param[in,out] A
 *         On entry, the m-by-n tile A.
 *         On exit, A has been set accordingly.
 *
 * @param[in] lda
 *         The leading dimension of the array A.  lda >= max(1,m).
 *
 ******************************************************************************/
__attribute__((weak))
void coreblas_zlaset(coreblas_enum_t uplo, int m, int n,
                 coreblas_complex64_t alpha, coreblas_complex64_t beta,
                 coreblas_complex64_t *A, int lda)
{
    if (alpha == 0.0 && beta == 0.0 && uplo == CoreBlasGeneral && m == lda) {
        // Use memset to zero continuous memory.
        memset((void*)A, 0, (size_t)m*n*sizeof(coreblas_complex64_t));
    }
    else {
    #ifdef COREBLAS_USE_64BIT_BLAS
        // Use LAPACKE_zlaset_work to initialize the matrix.
        LAPACKE_zlaset64_(LAPACK_COL_MAJOR, lapack_const(uplo),
                            m, n, alpha, beta, A, lda);
    #else
        // Use LAPACKE_zlaset_work to initialize the matrix.
        LAPACKE_zlaset_work(LAPACK_COL_MAJOR, lapack_const(uplo),
                            m, n, alpha, beta, A, lda);
    #endif 

    }
}