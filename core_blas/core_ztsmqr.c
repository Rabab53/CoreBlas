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
 * @ingroup core_tsmqr
 *
 *  Overwrites the general m1-by-n1 tile A1 and
 *  m2-by-n2 tile A2 with
 *
 *                                side = CoreBlasLeft        side = CoreBlasRight
 *    trans = CoreBlasNoTrans            Q * | A1 |           | A1 A2 | * Q
 *                                         | A2 |
 *
 *    trans = CoreBlas_ConjTrans       Q^H * | A1 |           | A1 A2 | * Q^H
 *                                         | A2 |
 *
 *  where Q is a complex unitary matrix defined as the product of k
 *  elementary reflectors
 *
 *    Q = H(1) H(2) . . . H(k)
 *
 *  as returned by coreblas_ztsqrt.
 *
 *******************************************************************************
 *
 * @param[in] side
 *         - CoreBlasLeft  : apply Q or Q^H from the Left;
 *         - CoreBlasRight :  apply Q or Q^H from the Right.
 *
 * @param[in] trans
 *         - CoreBlasNoTrans    : Apply Q;
 *         - CoreBlas_ConjTrans : Apply Q^H.
 *
 * @param[in] m1
 *         The number of rows of the tile A1. m1 >= 0.
 *
 * @param[in] n1
 *         The number of columns of the tile A1. n1 >= 0.
 *
 * @param[in] m2
 *         The number of rows of the tile A2. m2 >= 0.
 *         m2 = m1 if side == CoreBlasRight.
 *
 * @param[in] n2
 *         The number of columns of the tile A2. n2 >= 0.
 *         n2 = n1 if side == CoreBlasLeft.
 *
 * @param[in] k
 *         The number of elementary reflectors whose product defines
 *         the matrix Q.
 *
 * @param[in] ib
 *         The inner-blocking size.  ib >= 0.
 *
 * @param[in,out] A1
 *         On entry, the m1-by-n1 tile A1.
 *         On exit, A1 is overwritten by the application of Q.
 *
 * @param[in] lda1
 *         The leading dimension of the array A1. lda1 >= max(1,m1).
 *
 * @param[in,out] A2
 *         On entry, the m2-by-n2 tile A2.
 *         On exit, A2 is overwritten by the application of Q.
 *
 * @param[in] lda2
 *         The leading dimension of the tile A2. lda2 >= max(1,m2).
 *
 * @param[in] V
 *         The i-th row must contain the vector which defines the
 *         elementary reflector H(i), for i = 1,2,...,k, as returned by
 *         coreblas_ZTSQRT in the first k columns of its array argument V.
 *
 * @param[in] ldv
 *         The leading dimension of the array V. ldv >= max(1,k).
 *
 * @param[in] T
 *         The ib-by-k triangular factor T of the block reflector.
 *         T is upper triangular by block (economic storage);
 *         The rest of the array is not referenced.
 *
 * @param[in] ldt
 *         The leading dimension of the array T. ldt >= ib.
 *
 * @param work
 *         Auxiliary workspace array of length
 *         ldwork-by-n1 if side == CoreBlasLeft
 *         ldwork-by-ib if side == CoreBlasRight
 *
 * @param[in] ldwork
 *         The leading dimension of the array work.
 *             ldwork >= max(1,ib) if side == CoreBlasLeft
 *             ldwork >= max(1,m1) if side == CoreBlasRight
 *
 *******************************************************************************
 *
 * @retval CoreBlasSuccess successful exit
 * @retval < 0 if -i, the i-th argument had an illegal value
 *
 ******************************************************************************/
__attribute__((weak))
int coreblas_ztsmqr(coreblas_enum_t side, coreblas_enum_t trans,
                int m1, int n1, int m2, int n2, int k, int ib,
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
    if (m1 < 0) {
        coreblas_error("illegal value of m1");
        return -3;
    }
    if (n1 < 0) {
        coreblas_error("illegal value of n1");
        return -4;
    }
    if (m2 < 0 || (m2 != m1 && side == CoreBlasRight)) {
        coreblas_error("illegal value of m2");
        return -5;
    }
    if (n2 < 0 || (n2 != n1 && side == CoreBlasLeft)) {
        coreblas_error("illegal value of n2");
        return -6;
    }
    if (k < 0 ||
        (side == CoreBlasLeft  && k > m1) ||
        (side == CoreBlasRight && k > n1)) {
        coreblas_error("illegal value of k");
        return -7;
    }
    if (ib < 0) {
        coreblas_error("illegal value of ib");
        return -8;
    }
    if (A1 == NULL) {
        coreblas_error("NULL A1");
        return -9;
    }
    if (lda1 < imax(1, m1)) {
        coreblas_error("illegal value of lda1");
        return -10;
    }
    if (A2 == NULL) {
        coreblas_error("NULL A2");
        return -11;
    }
    if (lda2 < imax(1, m2)) {
        coreblas_error("illegal value of lda2");
        return -12;
    }
    if (V == NULL) {
        coreblas_error("NULL V");
        return -13;
    }
    if (ldv < imax(1, side == CoreBlasLeft ? m2 : n2)) {
        coreblas_error("illegal value of ldv");
        return -14;
    }
    if (T == NULL) {
        coreblas_error("NULL T");
        return -15;
    }
    if (ldt < imax(1, ib)) {
        coreblas_error("illegal value of ldt");
        return -16;
    }
    if (work == NULL) {
        coreblas_error("NULL work");
        return -17;
    }
    if (ldwork < imax(1, side == CoreBlasLeft ? ib : m1)) {
        coreblas_error("illegal value of ldwork");
        return -18;
    }

    // quick return
    if (m1 == 0 || n1 == 0 || m2 == 0 || n2 == 0 || k == 0 || ib == 0)
        return CoreBlasSuccess;

    int i1, i3;

    if ((side == CoreBlasLeft  && trans != CoreBlasNoTrans) ||
        (side == CoreBlasRight && trans == CoreBlasNoTrans)) {
        i1 = 0;
        i3 = ib;
    }
    else {
        i1 = ((k-1)/ib)*ib;
        i3 = -ib;
    }

    for (int i = i1; i > -1 && i < k; i += i3) {
        int kb = imin(ib, k-i);
        int ic = 0;
        int jc = 0;
        int mi = m1;
        int ni = n1;

        if (side == CoreBlasLeft) {
            // H or H^H is applied to C(i:m,1:n).
            mi = m1 - i;
            ic = i;
        }
        else {
            // H or H^H is applied to C(1:m,i:n).
            ni = n1 - i;
            jc = i;
        }

        // Apply H or H^H (NOTE: coreblas_zparfb used to be core_ztsrfb).
        coreblas_zparfb(side, trans, CoreBlasForward, CoreBlasColumnwise,
                    mi, ni, m2, n2, kb, 0,
                    &A1[lda1*jc+ic], lda1,
                    A2, lda2,
                    &V[ldv*i], ldv,
                    &T[ldt*i], ldt,
                    work, ldwork);


    }

    return CoreBlasSuccess;
}