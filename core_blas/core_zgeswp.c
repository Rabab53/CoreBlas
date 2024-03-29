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
#include "coreblas_internal.h"
#include "coreblas_types.h"
#include "core_lapack.h"

#define A(m, n) (coreblas_complex64_t*)coreblas_tile_addr(A, m, n)

/******************************************************************************/
__attribute__((weak))
void coreblas_zgeswp(coreblas_enum_t colrow,
                 coreblas_desc_t A, int k1, int k2, const int *ipiv, int incx)
{
    //================
    // CoreBlasRowwise
    //================
    if (colrow == CoreBlasRowwise) {
        if (incx > 0) {
            for (int m = k1-1; m <= k2-1; m += incx) {
                if (ipiv[m]-1 != m) {
                    int m1 = m;
                    int m2 = ipiv[m]-1;

                    int lda1 = coreblas_tile_mmain(A, m1/A.mb);
                    int lda2 = coreblas_tile_mmain(A, m2/A.mb);

                    cblas_zswap(A.n,
                                A(m1/A.mb, 0) + m1%A.mb, lda1,
                                A(m2/A.mb, 0) + m2%A.mb, lda2);
                }
            }
        }
        else {
            for (int m = k2-1; m >= k1-1; m += incx) {
                if (ipiv[m]-1 != m) {
                    int m1 = m;
                    int m2 = ipiv[m]-1;

                    int lda1 = coreblas_tile_mmain(A, m1/A.mb);
                    int lda2 = coreblas_tile_mmain(A, m2/A.mb);

                    cblas_zswap(A.n,
                                A(m1/A.mb, 0) + m1%A.mb, lda1,
                                A(m2/A.mb, 0) + m2%A.mb, lda2);
                }
            }
        }
    }
    //===================
    // CoreBlasColumnwise
    //===================
    else {
        if (incx > 0) {
            for (int n = k1-1; n <= k2-1; n += incx) {
                if (ipiv[n]-1 != n) {
                    int n1 = n;
                    int n2 = ipiv[n]-1;

                    int lda0 = coreblas_tile_mmain(A, 0);

                    cblas_zswap(A.m,
                                A(0, n1/A.nb) + (n1%A.nb)*lda0, 1,
                                A(0, n2/A.nb) + (n2%A.nb)*lda0, 1);
                }
            }
        }
        else {
            for (int n = k2-1; n >= k1-1; n += incx) {
                if (ipiv[n]-1 != n) {
                    int n1 = n;
                    int n2 = ipiv[n]-1;

                    int lda0 = coreblas_tile_mmain(A, 0);

                    cblas_zswap(A.m,
                                A(0, n1/A.nb) + (n1%A.nb)*lda0, 1,
                                A(0, n2/A.nb) + (n2%A.nb)*lda0, 1);
                }
            }
        }
    }
}
