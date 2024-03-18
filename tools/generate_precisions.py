#! /usr/bin/env python
# -*- encoding: ascii -*-

"To be executed from the top most directory where 'tools/codegen.py' is available."

import os
import sys

Output_Files = False # show files to be generated but don't generate

def codegen(letters, filenames, fn_format):
    for filename in filenames.split():
        if Output_Files:
            os.system(sys.executable + " tools/codegen.py --output {}".format(fn_format.format(filename)))
            continue
        for letter in letters.split():
            os.system(sys.executable + " tools/codegen.py -p {} {}".format(letter, fn_format.format(filename)))

def main(argv):
    global Output_Files
    if "--output" in argv:
        Output_Files = True

    elif "--help" in argv or "-h" in argv:
        print("{} [--output]\n".format(argv[0]))
        print("--output  show files to be generated but don't generate")
        return 0

    codegen("s d c", "core_lapack_z coreblas_z", "include/{}.h")
    codegen("ds", "include/coreblas_zc.h", "{}")
    #codegen("s d c", "dzamax zgelqf zgemm zgbmm zgeqrf zgesdd zunglq zungqr zunmlq zunmqr zpotrf zpotrs zsymm zsyr2k zsyrk ztradd ztrmm ztrsm ztrtri zunglq zungqr zunmlq zunmqr zgbsv zgbtrf zgbtrs zgeadd zgeinv zgelqs zgels zgeqrs zgesv zgeswp zgetrf zgetri zgetrs zhemm zher2k zherk zhesv zhetrf zhetrs zlacpy zlangb zlange zlanhe zlansy zlantr zlascl zlaset zlauum zpbsv zpbtrf zpbtrs zpoinv zposv zpotri zgetri_aux zdesc2ge zdesc2pb zdesc2tr zge2desc zgb2desc zgbset zpb2desc ztr2desc pdzamax pzgbtrf pzgeadd pzgelqf pzgelqf_tree pzgemm pzgeqrf pzgeqrf_tree pzgeswp pzgetrf pzgetri_aux pzhemm pzher2k pzherk pzhetrf_aasen pzlacpy pzlangb pzlange pzlanhe pzlansy pzlantr pzlascl pzlaset pzlauum pzpbtrf pzpotrf pzsymm pzsyr2k pzsyrk pztbsm pztradd pztrmm pztrsm pztrtri pzunglq pzunglq_tree pzungqr pzungqr_tree pzunmlq pzunmlq_tree pzunmqr pzunmqr_tree pzdesc2ge pzdesc2pb pzdesc2tr pzge2desc pzgb2desc pzpb2desc pztr2desc pzge2gb pzgbbrd_static pzgecpy_tile2lapack_band pzlarft_blgtrd pzunmqr_blgtrd", "compute/{}.c")
    #codegen("s d", "zlaebz2 zlaneg2 zstevx2", "compute/{}.c")
    #codegen("ds", "zcposv zcgesv zcgbsv clag2z zlag2c pclag2z pzlag2c", "compute/{}.c")
    codegen("s d c", "zgeadd zgemm zgeswp zgetrf zheswp zlacpy zlacpy_band zheswp ztrsm dzamax zgelqt zgeqrt zgessq zhegst zhemm zher2k zherk zhessq zlange zlanhe zlansy zlantr zlascl zlaset zlauum zunmlq zunmqr zpemv zpamm zpotrf zhegst zsymm zsyr2k zsyrk zsyssq ztradd ztrmm ztrssq ztrtri ztslqt ztsmlq ztsmqr ztsqrt zttlqt zttmlq zttmqr zttqrt zunmlq zunmqr zparfb dcabs1 zlarfb_gemm zgbtype1cb zgbtype2cb zgbtype3cb", "core_blas/core_{}.c")
    codegen("ds", "zlag2c clag2z", "core_blas/core_{}.c")
    #codegen("s d c", "z.h", "test/test_{}")
    #codegen("s d", "zstevx2.c", "test/test_{}")
    #codegen("s d c", "dzamax zgbsv zgbtrf zgeadd zgeinv zgelqf zgelqs zgels zgemm zgbmm zgeqrf zgeqrs zgesv zgeswp zgetrf zgetri_aux zgetri zgetrs zhemm zher2k zherk zhesv zhetrf zlacpy zlangb zlange zlanhe zlansy zlantr zlascl zlaset zlauum zpbsv zpbtrf zpoinv zposv zpotrf zpotri zpotrs zsymm zsyr2k zsyrk ztradd ztrmm ztrsm ztrtri zunmlq zunmqr zgesdd", "test/test_{}.c")
    #codegen("ds", "zcposv zcgesv zcgbsv zlag2c clag2z", "test/test_{}.c")
    return 0

if "__main__" == __name__:
    sys.exit(main(sys.argv))
