# Multi-Arch C++ & WebAssembly Development Environment

The container images the CI of a family of C++ libraries runs in, published to
`ghcr.io/jfalcou/compilers`. One directory per image, one tag per build.

| Directory | Tag | Base | What it carries |
|---|---|---|---|
| `basic/` | `v11` | `ubuntu:resolute` | GCC 16, Clang 22, the cross toolchains, Emscripten, QEMU, Doxygen |
| `mpi/` | `mpi-v1` | `ghcr.io/jfalcou/compilers:v11` | the basic image plus OpenMPI |
| `sycl/` | `sycl-v1` | `archlinux/archlinux` | Intel oneAPI DPC++, for `icpx` and SYCL |
| `cuda/` | none in use | `nvcr.io/nvidia/cuda:12.3.1-devel-ubi8` | CUDA 12.3 with CMake and Ninja |
| `previous/` | not built | | the recipes of `v6`, `v7`, `v9`, `v9b` and `v10`, kept for reference |

## Contents of `basic/`

| Category | Tool | Version |
|---|---|---|
| C and C++ | GCC, G++ | 16 |
| C and C++ | Clang | 22, and `clang` and `clang++` point at it |
| Standard library | libc++ and libc++abi | 22 |
| Cross compilation | g++-16 | aarch64, armhf, riscv64, powerpc64, powerpc64le |
| Emulation | QEMU user mode, with binfmt | from apt |
| WebAssembly | Emscripten SDK | 5.0.5, pinned by tag |
| Documentation | Doxygen | 1.16.1, downloaded and checked against its sha256 |
| Coverage | gcovr | 7.x, pinned: 8 counts a header once per translation unit |
| Coverage | lcov | from apt |
| Build | CMake, Ninja, Make | from apt |
| Debug | GDB, Valgrind | from apt |
| Numerics | GMP, MPFR, MPFR C++ | from apt |
| Other | Boost headers, clang-format, lld, llvm, Python 3, git | from apt |

### Environment

```bash
EMSDK="/opt/wasm/emsdk"
EMSDK_VERSION="5.0.5"
EM_CONFIG="/opt/wasm/emsdk/.emscripten"
EMSDK_NODE="/opt/wasm/emsdk/node/current/bin/node"
LD_LIBRARY_PATH="/usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/"
```

## Contents of `mpi/`

`libopenmpi-dev` and `libopenmpi40`, on top of the tag its `BASE_TAG` argument names.

## Contents of `sycl/`

`intel-oneapi-dpcpp-cpp` and its runtime libraries, GCC, CMake, Ninja, Python, git, from Arch Linux.
A job sources `/opt/intel/oneapi/setvars.sh` before calling `icpx`.

## Building and publishing

Publishing is a manual **Container Images** dispatch: pick the image, give the tag, following the
existing scheme (`v11`, `sycl-v2`). `refresh` ignores the layer cache. A pull request builds only the
images whose own `Dockerfile` it touched, under `<image>-dry-run`, and publishes nothing.

By hand:

```bash
docker build -t compilers:local basic/
docker run -it --rm -v "$(pwd)":/workspace -w /workspace compilers:local
```

## Using an image


```yaml
container:
  image: ghcr.io/jfalcou/compilers:v11
```

A platform matrix names the tag alone:

```yaml
- { name: "gcc"  , preset: "gcc"  , image: "v11"      }
- { name: "icpx" , preset: "icpx" , image: "sycl-v1"  }
```
