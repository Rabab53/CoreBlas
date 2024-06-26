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
 * @ingroup core_unmqr
 *
 *  Overwrites the general m-by-n tile C with
 *
 *                                    side = CoreBlasLeft      side = CoreBlasRight
 *    trans = CoreBlasNoTrans              Q * C                  C * Q
 *    trans = CoreBlas_ConjTrans         Q^H * C                  C * Q^H
 *
 *  where Q is a unitary matrix defined as the product of k
 *  elementary reflectors
 *    \f[
 *        Q = H(1) H(2) ... H(k)
 *    \f]
 *  as returned by coreblas_zgeqrt. Q is of order m if side = CoreBlasLeft
 *  and of order n if side = CoreBlasRight.
 *
 *******************************************************************************
 *
 * @param[in] side
 *         - CoreBlasLeft  : apply Q or Q^H from the Left;
 *         - CoreBlasRight : apply Q or Q^H from the Right.
 *
 * @param[in] trans
 *         - CoreBlasNoTrans    :  No transpose, apply Q;
 *         - CoreBlas_ConjTrans :  Transpose, apply Q^H.
 *
 * @param[in] m
 *         The number of rows of the tile C.  m >= 0.
 *
 * @param[in] n
 *         The number of columns of the tile C.  n >= 0.
 *
 * @param[in] k
 *         The number of elementary reflectors whose product defines
 *         the matrix Q.
 *         If side = CoreBlasLeft,  m >= k >= 0;
 *         if side = CoreBlasRight, n >= k >= 0.
 *
 * @param[in] ib
 *         The inner-blocking size.  ib >= 0.
 *
 * @param[in] A
 *         Dimension:  (lda,k)
 *         The i-th column must contain the vector which defines the
 *         elementary reflector H(i), for i = 1,2,...,k,
 *         as returned by coreblas_zgeqrt in the first k columns of its
 *         array argument A.
 *
 * @param[in] lda
 *         The leading dimension of the array A.
 *         If side = CoreBlasLeft,  lda >= max(1,m);
 *         if side = CoreBlasRight, lda >= max(1,n).
 *
 * @param[in] T
 *         The ib-by-k triangular factor T of the block reflector.
 *         T is upper triangular by block (economic storage);
 *         The rest of the array is not referenced.
 *
 * @param[in] ldt
 *         The leading dimension of the array T. ldt >= ib.
 *
 * @param[in,out] C
 *         On entry, the m-by-n tile C.
 *         On exit, C is overwritten by Q*C or Q^T*C or C*Q^T or C*Q.
 *
 * @param[in] ldc
 *         The leading dimension of the array C. ldc >= max(1,m).
 *
 * @param work
 *         Auxiliary workspace array of length
 *         ldwork-by-n  if side == CoreBlasLeft
 *         ldwork-by-ib if side == CoreBlasRight
 *
 * @param[in] ldwork
 *         The leading dimension of the array work.
 *             ldwork >= max(1,ib) if side == CoreBlasLeft
 *             ldwork >= max(1,m)  if side == CoreBlasRight
 *
 *******************************************************************************
 *
 * @retval CoreBlasSuccess successful exit
 * @retval < 0 if -i, the i-th argument had an illegal value
 *
 ******************************************************************************/
__attribute__((weak))
int coreblas_zunmqr(coreblas_enum_t side, coreblas_enum_t trans,
                int m, int n, int k, int ib,
                const coreblas_complex64_t *A,    int lda,
                const coreblas_complex64_t *T,    int ldt,
                      coreblas_complex64_t *C,    int ldc,
                      coreblas_complex64_t *work, int ldwork)
{
    // Check input arguments.
    if (side != CoreBlasLeft && side != CoreBlasRight) {
        coreblas_error("illegal value of side");
        return -1;
    }

    int nq; // order of Q
    int nw; // dimension of work

    if (side == CoreBlasLeft) {
        nq = m;
        nw = n;
    }
    else {
        nq = n;
        nw = m;
    }

    if (trans != CoreBlasNoTrans && trans != CoreBlas_ConjTrans) {
        coreblas_error("illegal value of trans");
        return -2;
    }
    if (m < 0) {
        coreblas_error("illegal value of m");
        return -3;
    }
    if (n < 0) {
        coreblas_error("illegal value of n");
        return -4;
    }
    if (k < 0 || k > nq) {
        coreblas_error("illegal value of k");
        return -5;
    }
    if (ib < 0) {
        coreblas_error("illegal value of ib");
        return -6;
    }
    if (A == NULL) {
        coreblas_error("NULL A");
        return -7;
    }
    if (lda < imax(1, nq) && nq > 0) {
        coreblas_error("illegal value of lda");
        return -8;
    }
    if (T == NULL) {
        coreblas_error("NULL T");
        return -9;
    }
    if (ldt < imax(1, ib)) {
        coreblas_error("illegal value of ldt");
        return -10;
    }
    if (C == NULL) {
        coreblas_error("NULL C");
        return -11;
    }
    if (ldc < imax(1, m) && m > 0) {
        coreblas_error("illegal value of ldc");
        return -12;
    }
    if (work == NULL) {
        coreblas_error("NULL work");
        return -13;
    }
    if (ldwork < imax(1, nw) && nw > 0) {
        coreblas_error("illegal value of ldwork");
        return -14;
    }

    // quick return
    if (m == 0 || n == 0 || k == 0)
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
        int ni = n;
        int mi = m;

        if (side == CoreBlasLeft) {
            // H or H^H is applied to C(i:m,1:n).
            mi = m - i;
            ic = i;
        }
        else {
            // H or H^H is applied to C(1:m,i:n).
            ni = n - i;
            jc = i;
        }
#ifdef COREBLAS_USE_64BIT_BLAS
        // Apply H or H^H.
        LAPACKE_zlarfb64_(LAPACK_COL_MAJOR,
                            lapack_const(side),
                            lapack_const(trans),
                            lapack_const(CoreBlasForward),
                            lapack_const(CoreBlasColumnwise),
                            mi, ni, kb,
                            &A[lda*i+i], lda,
                            &T[ldt*i], ldt,
                            &C[ldc*jc+ic], ldc);
#else
        // Apply H or H^H.
        LAPACKE_zlarfb(LAPACK_COL_MAJOR,
                            lapack_const(side),
                            lapack_const(trans),
                            lapack_const(CoreBlasForward),
                            lapack_const(CoreBlasColumnwise),
                            mi, ni, kb,
                            &A[lda*i+i], lda,
                            &T[ldt*i], ldt,
                            &C[ldc*jc+ic], ldc);
#endif

    }

    return CoreBlasSuccess;
}
