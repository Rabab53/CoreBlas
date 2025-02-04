# CoreBlas
This repository contains the source files for the core blas implementation of the PLASMA library.


# libcoreblas Build Instructions

This guide explains how to build the `libcoreblas` binary to be used within Julia. The following instructions will help you compile, deploy, and develop the library seamlessly in your Julia environment.

## Build Script

```julia
# Note that this script can accept some limited command-line arguments, run
# `julia build_tarballs.jl --help` to see a usage message.
using BinaryBuilder, Pkg

name = "libcoreblas"
version = v"23.8.2"

# Collection of sources required to complete build
sources = [
    GitSource("https://github.com/Rabab53/CoreBlas.git", "329f6b84f9c197d47a4f1e99d37b17be1720e120")
]

# Bash recipe for building across all platforms
script = raw"""
cd $WORKSPACE/srcdir
cd CoreBlas/
cmake -B build -DCMAKE_INSTALL_PREFIX=$prefix -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TARGET_TOOLCHAIN} -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel ${nproc}
cmake --install build
logout
"""

# These are the platforms we will build for by default, unless further
# platforms are passed in on the command line
platforms = [
    Platform("x86_64", "linux"; libc = "glibc")
]

# The products that we will ensure are always built
products = [
    LibraryProduct("libcoreblas", :libcoreblas)
]

# Dependencies that must be installed before this package can be built
dependencies = [
    Dependency("OpenBLAS32_jll")
]

# Build the tarballs, and possibly a `build.jl` as well.
build_tarballs(ARGS, name, version, sources, script, platforms, products, dependencies; julia_compat="1.10")
```

## Build Steps

1. Run the build script:
   ```bash
   julia --project=. build_coreblas.jl --deploy=local x86_64-linux-gnu
   ```

2. After the build completes, you will find the shared library located at:
   ```
   ~/julia/dev/libcoreblas_jll/
   ```

3. To develop the library in Julia, use the following command:
   ```julia
   ] dev ~/julia/dev/libcoreblas_jll/
   ```

This will add the local package to your Julia environment, allowing you to work with `libcoreblas` directly.

## Example: Wrapping `coreblas_dgbtype1cb` in Julia

Here's an example of how to wrap the `coreblas_dgbtype1cb` function from `libcoreblas`:

```julia
using LinearAlgebra, libcoreblas_jll, OpenBLAS32_jll

function coreblas_gbtype1cb!(::Type{T}, n, nb, A::AbstractMatrix{T}, 
    VQ::Vector{T}, TAUQ::Vector{T}, VP::Vector{T}, TAUP::Vector{T}) where {T<: Number}
    m1, n1 = size(A)
    nq = size(VQ)
    np = size(VP)
    uplo = 121
    nb = nb
    st = 0
    ed = 4
    sweep = 1
    Vblksiz = 1
    wantz = 0
    work = Vector{T}(undef, nb)
    
    @show m1, n1

    ccall((:coreblas_dgbtype1cb, "libcoreblas.so"), Cvoid,
        (Int64, Int64, Int64, Ptr{T}, Int64, Ptr{T}, Ptr{T},
         Ptr{T}, Ptr{T}, Int64, Int64, Int64, Int64, Int64, Ptr{T}),
        uplo, n, nb, A, m1, VQ, TAUQ, VP, TAUP, st, ed, sweep, Vblksiz, wantz, work)
end

# Example usage
n = 4
nb = 2
A = rand(3 * nb + 1, n)
VP = zeros(n)
VQ = zeros(n)
TAUP = zeros(n)
TAUQ = zeros(n)

coreblas_gbtype1cb!(Float64, n, nb, A, VQ, TAUQ, VP, TAUP)

# Display results
display(A)
display(VP)
display(VQ)
display(TAUP)
display(TAUQ)
```

This example demonstrates how to initialize the required data structures, call the wrapped function, and display the results after execution.
