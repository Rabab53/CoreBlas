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
 * @ingroup core_trmm
 *
 *  Performs a triangular matrix-matrix multiply of the form
 *
 *          \f[B = \alpha [op(A) \times B] \f], if side = CoreBlasLeft  or
 *          \f[B = \alpha [B \times op(A)] \f], if side = CoreBlasRight
 *
 *  where op( X ) is one of:
 *
 *          - op(A) = A   or
 *          - op(A) = A^T or
 *          - op(A) = A^H
 *
 *  alpha is a scalar, B is an m-by-n matrix and A is a unit or non-unit, upper
 *  or lower triangular matrix.
 *
 *******************************************************************************
 *
 * @param[in] side
 *          Specifies whether op( A ) appears on the left or on the right of B:
 *          - CoreBlasLeft:  alpha*op( A )*B
 *          - CoreBlasRight: alpha*B*op( A )
 *
 * @param[in] uplo
 *          Specifies whether the matrix A is upper triangular or lower
 *          triangular:
 *          - CoreBlasUpper: Upper triangle of A is stored;
 *          - CoreBlasLower: Lower triangle of A is stored.
 *
 * @param[in] transa
 *          Specifies whether the matrix A is transposed, not transposed or
 *          conjugate transposed:
 *          - CoreBlasNoTrans:   A is transposed;
 *          - CoreBlasTrans:     A is not transposed;
 *          - CoreBlasConjTrans: A is conjugate transposed.
 *
 * @param[in] diag
 *          Specifies whether or not A is unit triangular:
 *          - CoreBlasNonUnit: A is non-unit triangular;
 *          - CoreBlasUnit:    A is unit triangular.
 *
 * @param[in] m
 *          The number of rows of matrix B.
 *          m >= 0.
 *
 * @param[in] n
 *          The number of columns of matrix B.
 *          n >= 0.
 *
 * @param[in] alpha
 *          The scalar alpha.
 *
 * @param[in] A
 *          The triangular matrix A of dimension lda-by-k, where k is m when
 *          side='L' or 'l' and k is n when when side='R' or 'r'. If uplo =
 *          CoreBlasUpper, the leading k-by-k upper triangular part of the array
 *          A contains the upper triangular matrix, and the strictly lower
 *          triangular part of A is not referenced. If uplo = CoreBlasLower, the
 *          leading k-by-k lower triangular part of the array A contains the
 *          lower triangular matrix, and the strictly upper triangular part of
 *          A is not referenced. If diag = CoreBlasUnit, the diagonal elements of
 *          A are also not referenced and are assumed to be 1.
 *
 * @param[in] lda
 *          The leading dimension of the array A. When side='L' or 'l',
 *          lda >= max(1,m), when side='R' or 'r' then lda >= max(1,n).
 *
 * @param[in,out] B
 *          On entry, the matrix B of dimension ldb-by-n.
 *          On exit, the result of a triangular matrix-matrix multiply
 *          ( alpha*op(A)*B ) or ( alpha*B*op(A) ).
 *
 * @param[in] ldb
 *          The leading dimension of the array B. ldb >= max(1,m).
 *
 ******************************************************************************/
__attribute__((weak))
void coreblas_ztrmm(
    coreblas_enum_t side, coreblas_enum_t uplo,
    coreblas_enum_t transa, coreblas_enum_t diag,
    int m, int n,
    coreblas_complex64_t alpha, const coreblas_complex64_t *A, int lda,
                                    coreblas_complex64_t *B, int ldb)
{
    #ifdef COREBLAS_USE_64BIT_BLAS
        cblas_ztrmm64_(
            CblasColMajor,
            (CBLAS_SIDE)side, (CBLAS_UPLO)uplo,
            (CBLAS_TRANSPOSE)transa, (CBLAS_DIAG)diag,
            m, n,
            CBLAS_SADDR(alpha), A, lda, B, ldb);
    #else       
        cblas_ztrmm(
            CblasColMajor,
            (CBLAS_SIDE)side, (CBLAS_UPLO)uplo,
            (CBLAS_TRANSPOSE)transa, (CBLAS_DIAG)diag,
            m, n,
             CBLAS_SADDR(alpha), A, lda, B, ldb);
    #endif

}