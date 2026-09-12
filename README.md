# Multi-Arch C++ & WebAssembly Development Environment

The container images the CI of a family of C++ libraries runs in, published to
`ghcr.io/jfalcou/compilers`. One directory per image, one tag per build.

| Directory | Tag | Base | What it carries |
|---|---|---|---|
| `basic/` | `v11` | `ubuntu:resolute` | GCC 16, Clang 22, the cross toolchains, Emscripten, QEMU, Doxygen |
| `mpi/` | `mpi-v1` | `ghcr.io/jfalcou/compilers` | the basic image plus OpenMPI |
| `sycl/` | `sycl-v1` | `archlinux/archlinux` | Intel oneAPI DPC++, for `icpx` and SYCL |
| `cuda/` | none in use | `nvcr.io/nvidia/cuda:12.3.1-devel-ubi8` | CUDA 12.3 with CMake and Ninja |
| `previous/` | not built | | the recipes of `v6`, `v7`, `v9`, `v9b` and `v10`, kept for reference |

A tag is immutable: a new build takes the next one, and every consumer moves to it deliberately.
The Nvidia jobs of the libraries run on a self-hosted machine with its own toolchain, so nothing
pulls `cuda/` today.

## Contents of `basic/`

| Category | Tool | Version |
|---|---|---|
| C and C++ | GCC, G++ | 16; `g++-15` arrives underneath, required by `build-essential` |
| C and C++ | Clang | 22; `clang` and `clang++` point at it |
| Standard library | libc++ and libc++abi | 22; the packages conflict, so only one version can be installed |
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

Three things are pinned rather than taken from apt, and each is built in a stage of its own so a
change to the package list does not rebuild them: the Emscripten SDK, cloned at its tag, Doxygen,
whose archive is verified, and the gcovr version, which a smoke test refuses to see drift.

One version per family, the newest the distribution packages. The gcc crosses also carry the target sysroots that
clang uses for arm and riscv, which is why all five stay even though only aarch64 and powerpc64 are driven by a gcc
toolchain.

Ubuntu ships the GCC 16 trunk snapshot unstripped, and it is enormous: the aarch64 `cc1plus` is 341 MB against 37 MB
for the released 14. `strip --strip-debug` over `/usr/libexec/gcc` and `/usr/libexec/gcc-cross` takes those two
directories from 4671 MB down to 998 MB, and every compiler still builds afterwards. It runs inside the install `RUN`,
since a later layer would delete nothing.

The image ends on that smoke test: `node`, `emcc`, `gcovr` and the compilers are run once each, so a
toolchain that cannot start fails the build instead of reaching a consumer.

### Environment

```bash
EMSDK="/opt/wasm/emsdk"
EMSDK_VERSION="5.0.5"
EM_CONFIG="/opt/wasm/emsdk/.emscripten"
EMSDK_NODE="/opt/wasm/emsdk/node/current/bin/node"
LD_LIBRARY_PATH="/usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/"
```

`node/current` is a version-free symlink: the SDK moving to another Node release leaves `EMSDK_NODE`
valid. The dynamic loaders of the cross targets are symlinked into `/lib`, so a QEMU run finds them
without being told where they are.

`/github/workspace` is declared a safe directory for git, which is what a checkout inside a
container needs.

## Contents of `mpi/`

The basic image plus `libopenmpi-dev` and `libopenmpi40`. It lives apart because OpenMPI depends on gfortran, which
drags a second GCC, and on `libucx0`, which drags the ROCm chain: 375 MB that no CI job of the fleet uses.

## Contents of `sycl/`

Arch Linux, so the packages are whatever was current at build time: `intel-oneapi-dpcpp-cpp` and its
runtime libraries, GCC, CMake, Ninja, Python, git. A job using it sources
`/opt/intel/oneapi/setvars.sh` before calling `icpx`.

## Building and publishing

Publishing is manual, from the Actions tab: run **Container Images**, pick the image, and give the
tag it goes out under, following the existing scheme (`v11`, `sycl-v2`). `refresh` ignores the layer
cache, which is the only way to pick up new upstream packages, and costs the full build.

A pull request builds only the images whose own `Dockerfile` it touched, under a `<image>-dry-run`
tag, and never publishes anything else. Layers are cached in GHCR next to the image, so a pull
request that changes one line does not rebuild a toolchain from scratch.

Building one by hand is the same recipe:

```bash
docker build -t compilers:local basic/
docker run -it --rm -v "$(pwd)":/workspace -w /workspace compilers:local
```

## Using an image

A workflow names the image and the tag it wants:

```yaml
container:
  image: ghcr.io/jfalcou/compilers:v10
```

The libraries reach it through their platform matrices, where a row names the tag alone:

```yaml
- { name: "gcc"  , preset: "gcc"  , image: "v10"      }
- { name: "icpx" , preset: "icpx" , image: "sycl-v1"  }
```

A new tag therefore means a pass over the consumers: nothing follows it on its own.
