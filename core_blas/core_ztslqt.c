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



#undef REAL
#define COMPLEX

/***************************************************************************//**
 *
 * @ingroup core_tslqt
 *
 *  Computes an LQ factorization of a rectangular matrix
 *  formed by coupling side-by-side a complex m-by-m
 *  lower triangular tile A1 and a complex m-by-n tile A2:
 *
 *    | A1 A2 | = L * Q
 *
 *  The tile Q is represented as a product of elementary reflectors
 *
 *    Q = H(k)^H . . . H(2)^H H(1)^H, where k = min(m,n).
 *
 *  Each H(i) has the form
 *
 *    H(i) = I - tau * v * v^H
 *
 *  where tau is a complex scalar, and v is a complex vector with
 *  v(1:i-1) = 0 and v(i) = 1; v(i+1:n)^H is stored on exit in
 *  A2(i,1:n), and tau in tau(i).
 *
 *******************************************************************************
 *
 * @param[in] m
 *         The number of rows of the tile A1 and A2. m >= 0.
 *         The number of columns of the tile A1.
 *
 * @param[in] n
 *         The number of columns of the tile A2. n >= 0.
 *
 * @param[in] ib
 *         The inner-blocking size.  ib >= 0.
 *
 * @param[in,out] A1
 *         On entry, the m-by-m tile A1.
 *         On exit, the elements on and below the diagonal of the array
 *         contain the m-by-m lower trapezoidal tile L;
 *         the elements above the diagonal are not referenced.
 *
 * @param[in] lda1
 *         The leading dimension of the array A1. lda1 >= max(1,m).
 *
 * @param[in,out] A2
 *         On entry, the m-by-n tile A2.
 *         On exit, all the elements with the array tau, represent
 *         the unitary tile Q as a product of elementary reflectors
 *         (see Further Details).
 *
 * @param[in] lda2
 *         The leading dimension of the tile A2. lda2 >= max(1,m).
 *
 * @param[out] T
 *         The ib-by-m triangular factor T of the block reflector.
 *         T is upper triangular by block (economic storage);
 *         The rest of the array is not referenced.
 *
 * @param[in] ldt
 *         The leading dimension of the array T. ldt >= ib.
 *
 * @param tau
 *         Auxiliarry workspace array of length m.
 *
 * @param work
 *         Auxiliary workspace array of length ib*m.
 *
 *******************************************************************************
 *
 * @retval CoreBlasSuccess successful exit
 * @retval < 0 if -i, the i-th argument had an illegal value
 *
 ******************************************************************************/
__attribute__((weak))
int coreblas_ztslqt(int m, int n, int ib,
                coreblas_complex64_t *A1, int lda1,
                coreblas_complex64_t *A2, int lda2,
                coreblas_complex64_t *T,  int ldt,
                coreblas_complex64_t *tau,
                coreblas_complex64_t *work)
{
    // Check input arguments.
    if (m < 0) {
        coreblas_error("illegal value of m");
        return -1;
    }
    if (n < 0) {
        coreblas_error("illegal value of n");
        return -2;
    }
    if (ib < 0) {
        coreblas_error("illegal value of ib");
        return -3;
    }
    if (A1 == NULL) {
        coreblas_error("NULL A1");
        return -4;
    }
    if (lda1 < imax(1, m) && m > 0) {
        coreblas_error("illegal value of lda1");
        return -5;
    }
    if (A2 == NULL) {
        coreblas_error("NULL A2");
        return -6;
    }
    if (lda2 < imax(1, m) && m > 0) {
        coreblas_error("illegal value of lda2");
        return -7;
    }
    if (T == NULL) {
        coreblas_error("NULL T");
        return -8;
    }
    if (ldt < imax(1, ib) && ib > 0) {
        coreblas_error("illegal value of ldt");
        return -9;
    }
    if (tau == NULL) {
        coreblas_error("NULL tau");
        return -10;
    }
    if (work == NULL) {
        coreblas_error("NULL work");
        return -11;
    }

    // quick return
    if (m == 0 || n == 0 || ib == 0)
        return CoreBlasSuccess;

    static coreblas_complex64_t zone  = 1.0;
    static coreblas_complex64_t zzero = 0.0;

    for (int ii = 0; ii < m; ii += ib) {
        int sb = imin(m-ii, ib);
        for (int i = 0; i < sb; i++) {
            // Generate elementary reflector H(ii*ib+i) to annihilate
            // A(ii*ib+i,ii*ib+i:n).
#ifdef COMPLEX
     #ifdef COREBLAS_USE_64BIT_BLAS
            LAPACKE_zlacgv64_(n, &A2[ii+i], lda2);
            LAPACKE_zlacgv64_(1, &A1[lda1*(ii+i)+ii+i], lda1);
     #else
            LAPACKE_zlacgv_work(n, &A2[ii+i], lda2);
            LAPACKE_zlacgv_work(1, &A1[lda1*(ii+i)+ii+i], lda1);
     #endif

#endif
    #ifdef COREBLAS_USE_64BIT_BLAS
            LAPACKE_zlarfg64_(n+1, &A1[lda1*(ii+i)+ii+i], &A2[ii+i], lda2,
                                &tau[ii+i]);
    #else
            LAPACKE_zlarfg_work(n+1, &A1[lda1*(ii+i)+ii+i], &A2[ii+i], lda2,
                                &tau[ii+i]);
    #endif


            coreblas_complex64_t alpha = -(tau[ii+i]);
            if (ii+i+1 < m) {
            #ifdef COREBLAS_USE_64BIT_BLAS
                            // Apply H(ii+i-1) to A(ii+i:ii+ib-1, ii+i-1:n) from the right.
                cblas_zcopy64_(sb-i-1,
                            &A1[lda1*(ii+i)+(ii+i+1)], 1,
                            work, 1);
                cblas_zgemv64_(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                            sb-i-1, n,
                            CBLAS_SADDR(zone), &A2[ii+i+1], lda2,
                            &A2[ii+i], lda2,
                            CBLAS_SADDR(zone), work, 1);

                cblas_zaxpy64_(sb-i-1, CBLAS_SADDR(alpha), work, 1,
                            &A1[lda1*(ii+i)+ii+i+1], 1);

                cblas_zgerc64_(CblasColMajor,
                            sb-i-1, n,
                            CBLAS_SADDR(alpha), work, 1,
                            &A2[ii+i], lda2,
                            &A2[ii+i+1], lda2);
            #else
                            // Apply H(ii+i-1) to A(ii+i:ii+ib-1, ii+i-1:n) from the right.
                cblas_zcopy(sb-i-1,
                            &A1[lda1*(ii+i)+(ii+i+1)], 1,
                            work, 1);
                cblas_zgemv(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                            sb-i-1, n,
                            CBLAS_SADDR(zone), &A2[ii+i+1], lda2,
                            &A2[ii+i], lda2,
                            CBLAS_SADDR(zone), work, 1);

                cblas_zaxpy(sb-i-1, CBLAS_SADDR(alpha), work, 1,
                            &A1[lda1*(ii+i)+ii+i+1], 1);

                cblas_zgerc(CblasColMajor,
                            sb-i-1, n,
                            CBLAS_SADDR(alpha), work, 1,
                            &A2[ii+i], lda2,
                            &A2[ii+i+1], lda2);
            #endif

            }
            #ifdef COREBLAS_USE_64BIT_BLAS
                // Calculate T.
                cblas_zgemv64_(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                        i, n,
                        CBLAS_SADDR(alpha), &A2[ii], lda2,
                                            &A2[ii+i], lda2,
                        CBLAS_SADDR(zzero), &T[ldt*(ii+i)], 1);
            #else
                // Calculate T.
                cblas_zgemv(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                        i, n,
                        CBLAS_SADDR(alpha), &A2[ii], lda2,
                                            &A2[ii+i], lda2,
                        CBLAS_SADDR(zzero), &T[ldt*(ii+i)], 1);
            #endif

#ifdef COMPLEX
    #ifdef COREBLAS_USE_64BIT_BLAS
            LAPACKE_zlacgv64_(n, &A2[ii+i], lda2);
            LAPACKE_zlacgv64_(1, &A1[lda1*(ii+i)+ii+i], lda1);
    #else
            LAPACKE_zlacgv_work(n, &A2[ii+i], lda2);
            LAPACKE_zlacgv_work(1, &A1[lda1*(ii+i)+ii+i], lda1);
    #endif

#endif
    #ifdef COREBLAS_USE_64BIT_BLAS
            cblas_ztrmv64_(
                CblasColMajor,
                (CBLAS_UPLO)CoreBlasUpper,
                (CBLAS_TRANSPOSE)CoreBlasNoTrans, (CBLAS_DIAG)CoreBlasNonUnit,
                i,
                &T[ldt*ii], ldt,
                &T[ldt*(ii+i)], 1);
    #else
            cblas_ztrmv(
                CblasColMajor,
                (CBLAS_UPLO)CoreBlasUpper,
                (CBLAS_TRANSPOSE)CoreBlasNoTrans, (CBLAS_DIAG)CoreBlasNonUnit,
                i,
                &T[ldt*ii], ldt,
                &T[ldt*(ii+i)], 1);    
    #endif


            T[ldt*(ii+i)+i] = tau[ii+i];
        }
        if (m > ii+sb) {
#ifdef COREBLAS_USE_64BIT_BLAS
            coreblas_ztsmlq64_(CoreBlasRight, CoreBlas_ConjTrans,
                        m-(ii+sb), sb, m-(ii+sb), n, ib, ib,
                        &A1[lda1*ii+ii+sb], lda1,
                        &A2[ii+sb], lda2,
                        &A2[ii], lda2,
                        &T[ldt*ii], ldt,
                        work, lda1);
#else
            coreblas_ztsmlq(CoreBlasRight, CoreBlas_ConjTrans,
                        m-(ii+sb), sb, m-(ii+sb), n, ib, ib,
                        &A1[lda1*ii+ii+sb], lda1,
                        &A2[ii+sb], lda2,
                        &A2[ii], lda2,
                        &T[ldt*ii], ldt,
                        work, lda1);
#endif

        }
    }

    return CoreBlasSuccess;
}