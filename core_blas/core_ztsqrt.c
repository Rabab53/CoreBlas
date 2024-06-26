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



// This will be swapped during the automatic code generation.
#undef REAL
#define COMPLEX

/***************************************************************************//**
 *
 * @ingroup core_tsqrt
 *
 * Computes a QR factorization of a rectangular matrix
 * formed by coupling an n-by-n upper triangular tile A1
 * on top of an m-by-n tile A2:
 *
 *    | A1 | = Q * R
 *    | A2 |
 *
 *******************************************************************************
 *
 * @param[in] m
 *         The number of columns of the tile A2. m >= 0.
 *
 * @param[in] n
 *         The number of rows of the tile A1.
 *         The number of columns of the tiles A1 and A2. n >= 0.
 *
 * @param[in] ib
 *         The inner-blocking size.  ib >= 0.
 *
 * @param[in,out] A1
 *         On entry, the n-by-n tile A1.
 *         On exit, the elements on and above the diagonal of the array
 *         contain the n-by-n upper trapezoidal tile R;
 *         the elements below the diagonal are not referenced.
 *
 * @param[in] lda1
 *         The leading dimension of the array A1. LDA1 >= max(1,N).
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
 *         The ib-by-n triangular factor T of the block reflector.
 *         T is upper triangular by block (economic storage);
 *         The rest of the array is not referenced.
 *
 * @param[in] ldt
 *         The leading dimension of the array T. ldt >= ib.
 *
 * @param tau
 *         Auxiliary workspace array of length n.
 *
 * @param work
 *         Auxiliary workspace array of length ib*n.
 *
 *******************************************************************************
 *
 * @retval CoreBlasSuccess successful exit
 * @retval < 0 if -i, the i-th argument had an illegal value
 *
 ******************************************************************************/
__attribute__((weak))
int coreblas_ztsqrt(int m, int n, int ib,
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

    for (int ii = 0; ii < n; ii += ib) {
        int sb = imin(n-ii, ib);
        for (int i = 0; i < sb; i++) {
            // Generate elementary reflector H( II*IB+I ) to annihilate
            // A( II*IB+I:M, II*IB+I ).
            #ifdef COREBLAS_USE_64BIT_BLAS
                LAPACKE_zlarfg64_(m+1, &A1[lda1*(ii+i)+ii+i], &A2[lda2*(ii+i)], 1,&tau[ii+i]);
            #else
                LAPACKE_zlarfg_work(m+1, &A1[lda1*(ii+i)+ii+i], &A2[lda2*(ii+i)], 1,&tau[ii+i]);
            #endif

            if (ii+i+1 < n) {
                // Apply H( II*IB+I ) to A( II*IB+I:M, II*IB+I+1:II*IB+IB )
                // from the left.
                coreblas_complex64_t alpha = -conj(tau[ii+i]);
            #ifdef COREBLAS_USE_64BIT_BLAS
                cblas_zcopy64_(sb-i-1, &A1[lda1*(ii+i+1)+(ii+i)], lda1, work, 1);
            #else
                cblas_zcopy(sb-i-1, &A1[lda1*(ii+i+1)+(ii+i)], lda1, work, 1);
            #endif
                
#ifdef COMPLEX
    #ifdef COREBLAS_USE_64BIT_BLAS
    LAPACKE_zlacgv64_(sb-i-1, work, 1);
    #else
    LAPACKE_zlacgv_work(sb-i-1, work, 1);
    #endif          
#endif

#ifdef COREBLAS_USE_64BIT_BLAS
    cblas_zgemv64_(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlas_ConjTrans,
                            m, sb-i-1,
                            CBLAS_SADDR(zone), &A2[lda2*(ii+i+1)], lda2,
                            &A2[lda2*(ii+i)], 1,
                            CBLAS_SADDR(zone), work, 1);

#else
    cblas_zgemv(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlas_ConjTrans,
                            m, sb-i-1,
                            CBLAS_SADDR(zone), &A2[lda2*(ii+i+1)], lda2,
                            &A2[lda2*(ii+i)], 1,
                            CBLAS_SADDR(zone), work, 1);
#endif

#ifdef COMPLEX
    #ifdef COREBLAS_USE_64BIT_BLAS
        LAPACKE_zlacgv64_(sb-i-1, work, 1);
    #else
        LAPACKE_zlacgv_work(sb-i-1, work, 1);
    #endif
#endif

#ifdef COREBLAS_USE_64BIT_BLAS
        cblas_zaxpy64_(sb-i-1, CBLAS_SADDR(alpha), work, 1,
                    &A1[lda1*(ii+i+1)+ii+i], lda1);
#else
        cblas_zaxpy(sb-i-1, CBLAS_SADDR(alpha), work, 1,
                    &A1[lda1*(ii+i+1)+ii+i], lda1);
#endif

#ifdef COMPLEX
    #ifdef COREBLAS_USE_64BIT_BLAS
        LAPACKE_zlacgv64_(sb-i-1, work, 1);
    #else
        LAPACKE_zlacgv_work(sb-i-1, work, 1);
    #endif
#endif

#ifdef COREBLAS_USE_64BIT_BLAS
        cblas_zgerc(CblasColMajor,
                            m, sb-i-1,
                            CBLAS_SADDR(alpha), &A2[lda2*(ii+i)], 1,
                            work, 1,
                            &A2[lda2*(ii+i+1)], lda2);
#else
        cblas_zgerc(CblasColMajor,
                            m, sb-i-1,
                            CBLAS_SADDR(alpha), &A2[lda2*(ii+i)], 1,
                            work, 1,
                            &A2[lda2*(ii+i+1)], lda2);
#endif

            }
            // Calculate T.
            coreblas_complex64_t alpha = -tau[ii+i];
            #ifdef COREBLAS_USE_64BIT_BLAS
                cblas_zgemv64_(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlas_ConjTrans,
                            m, i,
                            CBLAS_SADDR(alpha), &A2[lda2*ii], lda2,
                            &A2[lda2*(ii+i)], 1,
                            CBLAS_SADDR(zzero), &T[ldt*(ii+i)], 1);
    
                cblas_ztrmv64_(CblasColMajor, (CBLAS_UPLO)CoreBlasUpper,
                            (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                            (CBLAS_DIAG)CoreBlasNonUnit,
                            i,
                            &T[ldt*ii], ldt,
                            &T[ldt*(ii+i)], 1);

            #else
                cblas_zgemv(CblasColMajor, (CBLAS_TRANSPOSE)CoreBlas_ConjTrans,
                            m, i,
                            CBLAS_SADDR(alpha), &A2[lda2*ii], lda2,
                            &A2[lda2*(ii+i)], 1,
                            CBLAS_SADDR(zzero), &T[ldt*(ii+i)], 1);
    
                cblas_ztrmv(CblasColMajor, (CBLAS_UPLO)CoreBlasUpper,
                            (CBLAS_TRANSPOSE)CoreBlasNoTrans,
                            (CBLAS_DIAG)CoreBlasNonUnit,
                            i,
                            &T[ldt*ii], ldt,
                            &T[ldt*(ii+i)], 1);
            #endif

            T[ldt*(ii+i)+i] = tau[ii+i];
        }
        if (n > ii+sb) {

        coreblas_ztsmqr(CoreBlasLeft, CoreBlas_ConjTrans,
                        sb, n-(ii+sb), m, n-(ii+sb), ib, ib,
                        &A1[lda1*(ii+sb)+ii], lda1,
                        &A2[lda2*(ii+sb)], lda2,
                        &A2[lda2*ii], lda2,
                        &T[ldt*ii], ldt,
                        work, sb);

        }
    }

    return CoreBlasSuccess;
}