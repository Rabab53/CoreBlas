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
 * @ingroup core_symm
 *
 *  Performs one of the matrix-matrix operations
 *
 *     \f[ C = \alpha \times A \times B + \beta \times C \f]
 *  or
 *     \f[ C = \alpha \times B \times A + \beta \times C \f]
 *
 *  where alpha and beta are scalars, A is a symmetric matrix and B and
 *  C are m-by-n matrices.
 *
 *******************************************************************************
 *
 * @param[in] side
 *          Specifies whether the symmetric matrix A appears on the
 *          left or right in the operation as follows:
 *          - CoreBlasLeft:  \f[ C = \alpha \times A \times B + \beta \times C \f]
 *          - CoreBlasRight: \f[ C = \alpha \times B \times A + \beta \times C \f]
 *
 * @param[in] uplo
 *          Specifies whether the upper or lower triangular part of
 *          the symmetric matrix A is to be referenced as follows:
 *          - CoreBlasLower:     Only the lower triangular part of the
 *                             symmetric matrix A is to be referenced.
 *          - CoreBlasUpper:     Only the upper triangular part of the
 *                             symmetric matrix A is to be referenced.
 *
 * @param[in] m
 *          The number of rows of the matrix C. m >= 0.
 *
 * @param[in] n
 *          The number of columns of the matrix C. n >= 0.
 *
 * @param[in] alpha
 *          The scalar alpha.
 *
 * @param[in] A
 *          A is an lda-by-ka matrix, where ka is m when side = CoreBlasLeft,
 *          and is n otherwise. Only the uplo triangular part is referenced.
 *
 * @param[in] lda
 *          The leading dimension of the array A. lda >= max(1,ka).
 *
 * @param[in] B
 *          B is an ldb-by-n matrix, where the leading m-by-n part of
 *          the array B must contain the matrix B.
 *
 * @param[in] ldb
 *          The leading dimension of the array B. ldb >= max(1,m).
 *
 * @param[in] beta
 *          The scalar beta.
 *
 * @param[in,out] C
 *          C is an ldc-by-n matrix.
 *          On exit, the array is overwritten by the m-by-n updated matrix.
 *
 * @param[in] ldc
 *          The leading dimension of the array C. ldc >= max(1,m).
 *
 ******************************************************************************/
__attribute__((weak))
void coreblas_zsymm(coreblas_enum_t side, coreblas_enum_t uplo,
                int m, int n,
                coreblas_complex64_t alpha, const coreblas_complex64_t *A, int lda,
                                          const coreblas_complex64_t *B, int ldb,
                coreblas_complex64_t beta,        coreblas_complex64_t *C, int ldc)
{
    #ifdef COREBLAS_USE_64BIT_BLAS
        cblas_zsymm64_(CblasColMajor,
                (CBLAS_SIDE)side, (CBLAS_UPLO)uplo,
                m, n,
                CBLAS_SADDR(alpha), A, lda,
                                    B, ldb,
                CBLAS_SADDR(beta),  C, ldc);
    #else
        cblas_zsymm(CblasColMajor,
                (CBLAS_SIDE)side, (CBLAS_UPLO)uplo,
                m, n,
                CBLAS_SADDR(alpha), A, lda,
                                    B, ldb,
                CBLAS_SADDR(beta),  C, ldc);
    #endif

}

