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
 * @ingroup core_parfb
 *
 *  Applies an upper triangular block reflector H
 *  or its transpose H^H to a rectangular matrix formed by
 *  coupling two tiles A1 and A2. Matrix V is:
 *
 *          COLUMNWISE                    ROWWISE
 *
 *         |     K     |                 |      N2-L     |   L  |
 *      __ _____________ __           __ _________________        __
 *         |    |      |                 |               | \
 *         |    |      |                 |               |   \    L
 *    M2-L |    |      |              K  |_______________|_____\  __
 *         |    |      | M2              |                      |
 *      __ |____|      |                 |                      | K-L
 *         \    |      |              __ |______________________| __
 *       L   \  |      |
 *      __     \|______| __              |          N2          |
 *
 *         | L |  K-L  |
 *
 *******************************************************************************
 *
 * @param[in] side
 *         - CoreBlasLeft  : apply Q or Q^H from the Left;
 *         - CoreBlasRight : apply Q or Q^H from the Right.
 *
 * @param[in] trans
 *         - CoreBlasNoTrans    : Apply Q;
 *         - CoreBlas_ConjTrans : Apply Q^H.
 *
 * @param[in] direct
 *         Indicates how H is formed from a product of elementary
 *         reflectors
 *         - CoreBlasForward  : H = H(1) H(2) . . . H(k) (Forward)
 *         - CoreBlasBackward : H = H(k) . . . H(2) H(1) (Backward)
 *
 * @param[in] storev
 *         Indicates how the vectors which define the elementary
 *         reflectors are stored:
 *         - CoreBlasColumnwise
 *         - CoreBlasRowwise
 *
 * @param[in] m1
 *         The number of columns of the tile A1. m1 >= 0.
 *
 * @param[in] n1
 *         The number of rows of the tile A1. n1 >= 0.
 *
 * @param[in] m2
 *         The number of columns of the tile A2. m2 >= 0.
 *
 * @param[in] n2
 *         The number of rows of the tile A2. n2 >= 0.
 *
 * @param[in] k
 *         The order of the matrix T (= the number of elementary
 *         reflectors whose product defines the block reflector).
 *
 * @param[in] l
 *         The size of the triangular part of V
 *
 * @param[in,out] A1
 *         On entry, the m1-by-n1 tile A1.
 *         On exit, A1 is overwritten by the application of Q.
 *
 * @param[in] lda1
 *         The leading dimension of the array A1. lda1 >= max(1,n1).
 *
 * @param[in,out] A2
 *         On entry, the m2-by-n2 tile A2.
 *         On exit, A2 is overwritten by the application of Q.
 *
 * @param[in] lda2
 *         The leading dimension of the tile A2. lda2 >= max(1,n2).
 *
 * @param[in] V
 *         (ldv,k)  if storev = 'C'
 *         (ldv,m2) if storev = 'R' and side = 'L'
 *         (ldv,n2) if storev = 'R' and side = 'R'
 *         Matrix V.
 *
 * @param[in] ldv
 *         The leading dimension of the array V.
 *         If storev = 'C' and side = 'L', ldv >= max(1,m2);
 *         if storev = 'C' and side = 'R', ldv >= max(1,n2);
 *         if storev = 'R', ldv >= k.
 *
 * @param[out] T
 *         The triangular k-by-k matrix T in the representation of the
 *         block reflector.
 *         T is upper triangular by block (economic storage);
 *         The rest of the array is not referenced.
 *
 * @param[in] ldt
 *         The leading dimension of the array T. ldt >= k.
 *
 * @param[in,out] work
 *
 * @param[in] ldwork
 *         The leading dimension of the array work.
 *
 *******************************************************************************
 *
 * @retval CoreBlasSuccess successful exit
 * @retval < 0 if -i, the i-th argument had an illegal value
 *
 ******************************************************************************/
__attribute__((weak))
int coreblas_zparfb(coreblas_enum_t side, coreblas_enum_t trans,
                coreblas_enum_t direct, coreblas_enum_t storev,
                int m1, int n1, int m2, int n2, int k, int l,
                      coreblas_complex64_t *A1,   int lda1,
                      coreblas_complex64_t *A2,   int lda2,
                const coreblas_complex64_t *V,    int ldv,
                const coreblas_complex64_t *T,    int ldt,
                      coreblas_complex64_t *work, int ldwork)
{
    // Check input arguments.
    if (side != CoreBlasLeft && side != CoreBlasRight) {
        coreblas_error("illegal value of side");
        return -1;
    }
    if (trans != CoreBlasNoTrans && trans != CoreBlas_ConjTrans) {
        coreblas_error("illegal value of trans");
        return -2;
    }
    if (direct != CoreBlasForward && direct != CoreBlasBackward) {
        coreblas_error("illegal value of direct");
        return -3;
    }
    if (storev != CoreBlasColumnwise && storev != CoreBlasRowwise) {
        coreblas_error("illegal value of storev");
        return -4;
    }
    if (m1 < 0) {
        coreblas_error("illegal value of m1");
        return -5;
    }
    if (n1 < 0) {
        coreblas_error("illegal value of n1");
        return -6;
    }
    if (m2 < 0 || (side == CoreBlasRight && m1 != m2)) {
        coreblas_error("illegal value of m2");
        return -7;
    }
    if (n2 < 0 ||
        (side == CoreBlasLeft && n1 != n2)) {
        coreblas_error("illegal value of n2");
        return -8;
    }
    if (k < 0) {
        coreblas_error("illegal value of k");
        return -9;
    }
    if (l < 0) {
        coreblas_error("illegal value of l");
        return -10;
    }
    if (A1 == NULL) {
        coreblas_error("NULL A1");
        return -11;
    }
    if (lda1 < 0) {
        coreblas_error("illegal value of lda1");
        return -12;
    }
    if (A2 == NULL) {
        coreblas_error("NULL A2");
        return -13;
    }
    if (lda2 < 0) {
        coreblas_error("illegal value of lda2");
        return -14;
    }
    if (V == NULL) {
        coreblas_error("NULL V");
        return -15;
    }
    if (ldv < 0) {
        coreblas_error("illegal value of ldv");
        return -16;
    }
    if (T == NULL) {
        coreblas_error("NULL T");
        return -17;
    }
    if (ldt < 0) {
        coreblas_error("illegal value of ldt");
        return -18;
    }
    if (work == NULL) {
        coreblas_error("NULL work");
        return -19;
    }
    if (ldwork < 0) {
        coreblas_error("illegal value of ldwork");
        return -20;
    }

    // quick return
    if (m1 == 0 || n1 == 0 || m2 == 0 || n2 == 0 || k == 0)
        return CoreBlasSuccess;

    coreblas_complex64_t zone  =  1.0;
    coreblas_complex64_t zmone = -1.0;

    if (direct == CoreBlasForward) {
        //=============================
        // CoreBlasForward / CoreBlasLeft
        //=============================
        if (side == CoreBlasLeft) {
            // Form  H * A  or  H^H * A  where  A = ( A1 )
            //                                      ( A2 )

            // W = A1 + op(V) * A2
            coreblas_zpamm(CoreBlasW, CoreBlasLeft, storev,
                       k, n1, m2, l,
                       A1,   lda1,
                       A2,   lda2,
                       V,    ldv,
                       work, ldwork);
        #ifdef COREBLAS_USE_64BIT_BLAS
                    // W = op(T) * W
            cblas_ztrmm64_(CblasColMajor,
                        CblasLeft, CblasUpper,
                        (CBLAS_TRANSPOSE)trans, CblasNonUnit,
                        k, n2,
                        CBLAS_SADDR(zone), T,    ldt,
                                           work, ldwork);
        #else
                    // W = op(T) * W
            cblas_ztrmm(CblasColMajor,
                        CblasLeft, CblasUpper,
                        (CBLAS_TRANSPOSE)trans, CblasNonUnit,
                        k, n2,
                        CBLAS_SADDR(zone), T,    ldt,
                                           work, ldwork);
        #endif 


            // A1 = A1 - W
            for (int j = 0; j < n1; j++) {
            #ifdef COREBLAS_USE_64BIT_BLAS
                    cblas_zaxpy64_(k, CBLAS_SADDR(zmone),
                            &work[ldwork*j], 1,
                            &A1[lda1*j], 1);
            #else
                    cblas_zaxpy(k, CBLAS_SADDR(zmone),
                            &work[ldwork*j], 1,
                            &A1[lda1*j], 1);
            #endif 

            }

            // A2 = A2 - op(V) * W
            // W = V * W, A2 = A2 - W
            coreblas_zpamm(CoreBlasA2, CoreBlasLeft, storev,
                       m2, n2, k, l,
                       A1,   lda1,
                       A2,   lda2,
                       V,    ldv,
                       work, ldwork);
        }
        //==============================
        // CoreBlasForward / CoreBlasRight
        //==============================
        else {
            // Form  H * A  or  H^H * A  where A  = ( A1 A2 )

            // W = A1 + A2 * op(V)
            coreblas_zpamm(CoreBlasW, CoreBlasRight, storev,
                       m1, k, n2, l,
                       A1,   lda1,
                       A2,   lda2,
                       V,    ldv,
                       work, ldwork);
 
        #ifdef COREBLAS_USE_64BIT_BLAS
                    // W = W * op(T)
            cblas_ztrmm64_(CblasColMajor,
                        CblasRight, CblasUpper,
                        (CBLAS_TRANSPOSE)trans, CblasNonUnit,
                        m2, k,
                        CBLAS_SADDR(zone), T,    ldt,
                                           work, ldwork);
        #else
                    // W = W * op(T)
            cblas_ztrmm(CblasColMajor,
                        CblasRight, CblasUpper,
                        (CBLAS_TRANSPOSE)trans, CblasNonUnit,
                        m2, k,
                        CBLAS_SADDR(zone), T,    ldt,
                                           work, ldwork);
        #endif


            // A1 = A1 - W
            for (int j = 0; j < k; j++) {
            #ifdef COREBLAS_USE_64BIT_BLAS
                    cblas_zaxpy64_(m1, CBLAS_SADDR(zmone),
                            &work[ldwork*j], 1,
                            &A1[lda1*j], 1);
            #else
                    cblas_zaxpy(m1, CBLAS_SADDR(zmone),
                            &work[ldwork*j], 1,
                            &A1[lda1*j], 1);
            #endif 

            }

            // A2 = A2 - W * op(V)
            // W = W * V^H, A2 = A2 - W
            coreblas_zpamm(CoreBlasA2, CoreBlasRight, storev,
                       m2, n2, k, l,
                       A1,   lda1,
                       A2,   lda2,
                       V,    ldv,
                       work, ldwork);
        }
    }
    else {
        coreblas_error("Backward / Left or Right not implemented");
        return CoreBlasErrorNotSupported;
    }

    return CoreBlasSuccess;
}