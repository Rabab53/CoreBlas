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



/***************************************************************************//**
 *
 * @ingroup core_pemv
 *
 *  performs one of the matrix-vector operations
 *
 *     y = alpha*op(A)*x + beta*y
 *
 *  where  op(A) is one of
 *
 *     op(A) = A   or   op(A) = A^T   or   op(A) = A^H,
 *
 *  alpha and beta are scalars, x and y are vectors and A is a
 *  pentagonal matrix (see further details).
 *
 *
 *  Arguments
 *  ==========
 *
 * @param[in] storev
 *         - CoreBlasColumnwise :  array A stored columwise
 *         - CoreBlasRowwise    :  array A stored rowwise
 *
 * @param[in] trans
 *         - CoreBlasNoTrans   :  y := alpha*A*x   + beta*y.
 *         - CoreBlasTrans     :  y := alpha*A^T*x + beta*y.
 *         - CoreBlasConjTrans :  y := alpha*A^H*x + beta*y.
 *
 * @param[in] m
 *         Number of rows of the matrix A.
 *         m must be at least zero.
 *
 * @param[in] n
 *         Number of columns of the matrix A.
 *         n must be at least zero.
 *
 * @param[in] l
 *         Order of triangle within the matrix A (l specifies the shape
 *         of the matrix A; see further details).
 *
 * @param[in] alpha
 *         Scalar alpha.
 *
 * @param[in] A
 *         Array of size lda-by-n.  On entry, the leading m-by-n part
 *         of the array A must contain the matrix of coefficients.
 *
 * @param[in] lda
 *         Leading dimension of array A.
 *
 * @param[in] X
 *         On entry, the incremented array X must contain the vector x.
 *
 * @param[in] incx
 *         Increment for the elements of X. incx must not be zero.
 *
 * @param[in] beta
 *         Scalar beta.
 *
 * @param[in,out] Y
 *         On entry, the incremented array Y must contain the vector y.
 *
 * @param[out] incy
 *         Increment for the elements of Y. incy must not be zero.
 *
 * @param[out] work
 *         Workspace array of size at least l.
 *
 *  Further Details
 *  ===============
 *
 *               |     n    |
 *            _   ___________   _
 *               |          |
 *     A:        |          |
 *          m-l  |          |
 *               |          |  m
 *            _  |.....     |
 *               \    :     |
 *            l    \  :     |
 *            _      \:_____|  _
 *
 *               |  l | n-l |
 *
 *
 *******************************************************************************
 *
 * @retval CoreBlasSuccess successful exit
 * @retval < 0 if -i, the i-th argument had an illegal value
 *
 ******************************************************************************/
__attribute__((weak))
int coreblas_zpemv(coreblas_enum_t trans, int storev,
               int m, int n, int l,
               coreblas_complex64_t alpha,
               const coreblas_complex64_t *A, int lda,
               const coreblas_complex64_t *X, int incx,
               coreblas_complex64_t beta,
               coreblas_complex64_t *Y, int incy,
               coreblas_complex64_t *work)
{
    // Check input arguments.
    if ((trans != CoreBlasNoTrans) &&
        (trans != CoreBlasTrans)   &&
        (trans != CoreBlasConjTrans)) {
        coreblas_error("Illegal value of trans");
        return -1;
    }
    if ((storev != CoreBlasColumnwise) && (storev != CoreBlasRowwise)) {
        coreblas_error("Illegal value of storev");
        return -2;
    }
    if (!( ((storev == CoreBlasColumnwise) && (trans != CoreBlasNoTrans)) ||
           ((storev == CoreBlasRowwise) && (trans == CoreBlasNoTrans)) )) {
        coreblas_error("Illegal values of trans/storev");
        return -2;
    }
    if (m < 0) {
        coreblas_error("Illegal value of m");
        return -3;
    }
    if (n < 0) {
        coreblas_error("Illegal value of n");
        return -4;
    }
    if (l > imin(m, n)) {
        coreblas_error("Illegal value of l");
        return -5;
    }
    if (lda < imax(1, m)) {
        coreblas_error("Illegal value of lda");
        return -8;
    }
    if (incx < 1) {
        coreblas_error("Illegal value of incx");
        return -10;
    }
    if (incy < 1) {
        coreblas_error("Illegal value of incy");
        return -13;
    }

    // quick return
    if ((m == 0) || (n == 0))
        return CoreBlasSuccess;
    if ((alpha == 0.0) && (beta == 0.0))
        return CoreBlasSuccess;

    // If l < 2, there is no triangular part.
    if (l == 1) l = 0;

    // Columnwise
    if (storev == CoreBlasColumnwise) {
        //        ______________
        //        |      |     |    A1: A[0]
        //        |      |     |    A2: A[m-l]
        //        |  A1  |     |    A3: A[(n-l)*lda]
        //        |      |     |
        //        |______| A3  |
        //        \      |     |
        //          \ A2 |     |
        //            \  |     |
        //              \|_____|

        // Columnwise / NoTrans
        if (trans == CoreBlasNoTrans) {
            coreblas_error("CoreBlasNoTrans/CoreBlasColumnwise not implemented");
            return -1;
        }
        // Columnwise / [Conj]Trans
        else {
            // l top rows of y
            if (l > 0) {
                // w = A_2^H * x_2
            #ifdef COREBLAS_USE_64BIT_BLAS
                cblas_zcopy64_(l, &X[incx*(m-l)], incx, work, 1);
                cblas_ztrmv64_(CblasColMajor, (CBLAS_UPLO)CoreBlasUpper,
                            (CBLAS_TRANSPOSE)trans,
                            (CBLAS_DIAG)CoreBlasNonUnit,
                            l, &A[m-l], lda, work, 1);
            #else
                cblas_zcopy(l, &X[incx*(m-l)], incx, work, 1);
                cblas_ztrmv(CblasColMajor, (CBLAS_UPLO)CoreBlasUpper,
                            (CBLAS_TRANSPOSE)trans,
                            (CBLAS_DIAG)CoreBlasNonUnit,
                            l, &A[m-l], lda, work, 1);
            #endif 


                if (m > l) {
                #ifdef COREBLAS_USE_64BIT_BLAS
                    // y_1 = beta * y_1 [ + alpha * A_1 * x_1 ]
                    cblas_zgemv64_(CblasColMajor, (CBLAS_TRANSPOSE)trans,
                                m-l, l, CBLAS_SADDR(alpha), A, lda,
                                X, incx, CBLAS_SADDR(beta), Y, incy);

                    // y_1 = y_1 + alpha * w
                    cblas_zaxpy64_(l, CBLAS_SADDR(alpha), work, 1, Y, incy);
                #else
                    // y_1 = beta * y_1 [ + alpha * A_1 * x_1 ]
                    cblas_zgemv(CblasColMajor, (CBLAS_TRANSPOSE)trans,
                                m-l, l, CBLAS_SADDR(alpha), A, lda,
                                X, incx, CBLAS_SADDR(beta), Y, incy);

                    // y_1 = y_1 + alpha * w
                    cblas_zaxpy(l, CBLAS_SADDR(alpha), work, 1, Y, incy);
                #endif 

                }
                else {
                    // y_1 = y_1 + alpha * w
                    if (beta == 0.0) {
                    #ifdef COREBLAS_USE_64BIT_BLAS
                        cblas_zscal64_(l, CBLAS_SADDR(alpha), work, 1);
                        cblas_zcopy64_(l, work, 1, Y, incy);
                    #else
                        cblas_zscal(l, CBLAS_SADDR(alpha), work, 1);
                        cblas_zcopy(l, work, 1, Y, incy);
                    #endif 

                    }
                    else {
                    #ifdef COREBLAS_USE_64BIT_BLAS
                        cblas_zscal64_(l, CBLAS_SADDR(beta), Y, incy);
                        cblas_zaxpy64_(l, CBLAS_SADDR(alpha), work, 1, Y, incy);
                    #else
                        cblas_zscal(l, CBLAS_SADDR(beta), Y, incy);
                        cblas_zaxpy(l, CBLAS_SADDR(alpha), work, 1, Y, incy);
                    #endif 

                    }
                }
            }

            // n-l bottom rows of Y
            if (n > l) {
                int k = n - l;
            #ifdef COREBLAS_USE_64BIT_BLAS
                cblas_zgemv64_(CblasColMajor, (CBLAS_TRANSPOSE)trans,
                            m, k, CBLAS_SADDR(alpha), &A[lda*l], lda,
                            X, incx, CBLAS_SADDR(beta), &Y[incy*l], incy);
            #else
                cblas_zgemv(CblasColMajor, (CBLAS_TRANSPOSE)trans,
                            m, k, CBLAS_SADDR(alpha), &A[lda*l], lda,
                            X, incx, CBLAS_SADDR(beta), &Y[incy*l], incy);
            #endif 

            }
        }
    }
    // Rowwise
    else {
        // --------------
        // |            | \           A1:  A[0]
        // |    A1      |   \         A2:  A[(n-l) * lda]
        // |            | A2  \       A3:  A[l]
        // |--------------------|
        // |        A3          |
        // ----------------------

        // Rowwise / NoTrans
        if (trans == CoreBlasNoTrans) {
            // l top rows of A and y
            if (l > 0) {
                #ifdef COREBLAS_USE_64BIT_BLAS
                                // w = A_2 * x_2
                    cblas_zcopy64_(l, &X[incx*(n-l)], incx, work, 1);
                    cblas_ztrmv64_(CblasColMajor, (CBLAS_UPLO)CoreBlasLower,
                            (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                            (CBLAS_DIAG)CoreBlasNonUnit,
                            l, &A[lda*(n-l)], lda, work, 1);
                #else
                                // w = A_2 * x_2
                    cblas_zcopy(l, &X[incx*(n-l)], incx, work, 1);
                    cblas_ztrmv(CblasColMajor, (CBLAS_UPLO)CoreBlasLower,
                            (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                            (CBLAS_DIAG)CoreBlasNonUnit,
                            l, &A[lda*(n-l)], lda, work, 1);
                #endif 


                if (n > l) {
                #ifdef COREBLAS_USE_64BIT_BLAS
                    // y_1 = beta * y_1 [ + alpha * A_1 * x_1 ]
                    cblas_zgemv64_(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                                l, n-l, CBLAS_SADDR(alpha), A, lda,
                                X, incx, CBLAS_SADDR(beta), Y, incy);

                    // y_1 = y_1 + alpha * w
                    cblas_zaxpy64_(l, CBLAS_SADDR(alpha), work, 1, Y, incy);
                #else
                                    // y_1 = beta * y_1 [ + alpha * A_1 * x_1 ]
                    cblas_zgemv(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                                l, n-l, CBLAS_SADDR(alpha), A, lda,
                                X, incx, CBLAS_SADDR(beta), Y, incy);

                    // y_1 = y_1 + alpha * w
                    cblas_zaxpy(l, CBLAS_SADDR(alpha), work, 1, Y, incy);
                #endif 

                }
                else {
                    // y_1 = y_1 + alpha * w
                    if (beta == 0.0) {
                    #ifdef COREBLAS_USE_64BIT_BLAS
                        cblas_zscal64_(l, CBLAS_SADDR(alpha), work, 1);
                        cblas_zcopy64_(l, work, 1, Y, incy);
                    #else
                        cblas_zscal(l, CBLAS_SADDR(alpha), work, 1);
                        cblas_zcopy(l, work, 1, Y, incy);
                    #endif 

                    }
                    else {
                    #ifdef COREBLAS_USE_64BIT_BLAS
                        cblas_zscal64_(l, CBLAS_SADDR(beta), Y, incy);
                        cblas_zaxpy64_(l, CBLAS_SADDR(alpha), work, 1, Y, incy);
                    #else
                        cblas_zscal(l, CBLAS_SADDR(beta), Y, incy);
                        cblas_zaxpy(l, CBLAS_SADDR(alpha), work, 1, Y, incy);
                    #endif 

                    }
                }
            }

            // m-l bottom rows of Y
            if (m > l) {
            #ifdef COREBLAS_USE_64BIT_BLAS
                cblas_zgemv64_(
                        CblasColMajor, (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                        m-l, n, CBLAS_SADDR(alpha), &A[l], lda,
                        X, incx, CBLAS_SADDR(beta), &Y[incy*l], incy);
            #else
                cblas_zgemv(
                        CblasColMajor, (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                        m-l, n, CBLAS_SADDR(alpha), &A[l], lda,
                        X, incx, CBLAS_SADDR(beta), &Y[incy*l], incy);
            #endif 

            }
        }
        // Rowwise/[Conj]Trans
        else {
            coreblas_error("CoreBlas[Conj]Trans/CoreBlasRowwise not implemented");
            return -1;
        }
    }

    return CoreBlasSuccess;
}
