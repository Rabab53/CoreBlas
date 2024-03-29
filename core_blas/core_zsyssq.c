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

#include <math.h>

/******************************************************************************/
__attribute__((weak))
void coreblas_zsyssq(coreblas_enum_t uplo,
                 int n,
                 const coreblas_complex64_t *A, int lda,
                 double *scale, double *sumsq)
{
    *scale = 0.0;
    *sumsq = 1.0;

    int ione = 1;
    if (uplo == CoreBlasUpper) {
        for (int j = 1; j < n; j++)
            // TODO: Inline this operation.
            #ifdef COREBLAS_USE_64BIT_BLAS
                LAPACK_zlassq64_(&j, &A[lda*j], &ione, scale, sumsq);
            #else
                LAPACK_zlassq(&j, &A[lda*j], &ione, scale, sumsq);
            #endif
            
    }
    else { // CoreBlasLower
        for (int j = 0; j < n-1; j++) {
            int len = n-j-1;
            // TODO: Inline this operation.
            #ifdef COREBLAS_USE_64BIT_BLAS
                LAPACK_zlassq64_(&len, &A[lda*j+j+1], &ione, scale, sumsq);
            #else
                LAPACK_zlassq(&len, &A[lda*j+j+1], &ione, scale, sumsq);
            #endif
        }
    }
    *sumsq *= 2.0;
    for (int i = 0; i < n; i++) {
        // diagonal is complex, don't ignore complex part
        double absa = cabs(A[lda*i+i]);
        if (absa != 0.0) { // != propagates nan
            if (*scale < absa) {
                *sumsq = 1.0 + *sumsq*((*scale/absa)*(*scale/absa));
                *scale = absa;
            }
            else {
                *sumsq = *sumsq + ((absa/(*scale))*(absa/(*scale)));
            }
        }
    }
}

/******************************************************************************/
void coreblas_zsyssq_aux(int m, int n,
                         const double *scale, const double *sumsq,
                         double *value)
{

    double scl = 0.0;
    double sum = 1.0;
    for (int j = 0; j < n; j++) {
        for (int i = j+1; i < n; i++) {
            int idx = m*j+i;
            if (scl < scale[idx]) {
                sum = sumsq[idx] +
                    sum*((scl/scale[idx])*(scl/scale[idx]));
                scl = scale[idx];
            }
            else if (scl > 0.) {
                sum = sum +
                    sumsq[idx]*((scale[idx]/scl)*(scale[idx]/scl));
            }
        }
    }
    sum = 2.0*sum;
    for (int j = 0; j < n; j++) {
        int idx = m*j+j;
        if (scl < scale[idx]) {
            sum = sumsq[idx] + sum*((scl/scale[idx])*(scl/scale[idx]));
            scl = scale[idx];
        }
        else if (scl > 0.) {
            sum = sum + sumsq[idx]*((scale[idx]/scl)*(scale[idx]/scl));
        }
    }
    *value = scl*sqrt(sum);
}
