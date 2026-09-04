# Multi-Arch C++ & WebAssembly Development Environment

The container images the CI of a family of C++ libraries runs in, published to
`ghcr.io/jfalcou/compilers`. One directory per image, one tag per build.

| Directory | Tag | Base | What it carries |
|---|---|---|---|
| `basic/` | `v10` | `ubuntu:questing` | GCC 14, Clang 19 to 21, the cross toolchains, Emscripten, QEMU, Doxygen |
| `sycl/` | `sycl-v1` | `archlinux/archlinux` | Intel oneAPI DPC++, for `icpx` and SYCL |
| `cuda/` | none in use | `nvcr.io/nvidia/cuda:12.3.1-devel-ubi8` | CUDA 12.3 with CMake and Ninja |
| `previous/` | not built | | the recipes of `v6`, `v7`, `v9` and `v9b`, kept for reference |

A tag is immutable: a new build takes the next one, and every consumer moves to it deliberately.
The Nvidia jobs of the libraries run on a self-hosted machine with its own toolchain, so nothing
pulls `cuda/` today.

## What `basic/` carries

| Category | Tool | Version |
|---|---|---|
| C and C++ | GCC, G++ | 14, with multilib |
| C and C++ | Clang | 19, 20 and 21; `clang` and `clang++` point at 19 |
| Standard library | libc++ and libc++abi | 19, 20 and 21 |
| Cross compilation | g++-14 | aarch64, armhf, riscv64, powerpc64, powerpc64le |
| Emulation | QEMU user mode, with binfmt | from apt |
| WebAssembly | Emscripten SDK | 5.0.5, pinned by tag |
| Documentation | Doxygen | 1.16.1, downloaded and checked against its sha256 |
| Coverage | gcovr | 7.x, pinned: 8 counts a header once per translation unit |
| Coverage | lcov | from apt |
| Build | CMake, Ninja, Make | from apt |
| Debug | GDB, Valgrind | from apt |
| Numerics | GMP, MPFR, MPFR C++ | from apt |
| Parallel | OpenMPI | from apt |
| Other | Boost headers, clang-format, lld, llvm, Python 3, git | from apt |

Three things are pinned rather than taken from apt, and each is built in a stage of its own so a
change to the package list does not rebuild them: the Emscripten SDK, cloned at its tag, Doxygen,
whose archive is verified, and the gcovr version, which a smoke test refuses to see drift.

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

`/github/workspace` is declared a safe directory for git, which is what a checkout inside a container
needs.

## What `sycl/` carries

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
