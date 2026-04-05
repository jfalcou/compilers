# Multi-Arch C++ & WebAssembly Development Environment

## Base Version

This Docker image provides a high-performance, multi-architecture C++ development and cross-compilation environment based on **Ubuntu Questing**.

### 🚀 Key Features

* **Modern Compilers:** Includes **GCC 14** and **Clang 19/20** with support for the latest C++ standards.
* **WebAssembly (Wasm):** Pre-configured **Emscripten SDK (v5.0.5)**.
* **Cross-Compilation:** Multi-arch support for `aarch64`, `armhf`, `powerpc64le`, and `riscv64`.
* **Emulation & Analysis:** **QEMU** for cross-arch execution and **Intel SDE** for instruction set simulation.

### 🛠 Software Inventory & Versions

#### Primary Toolchains
| Category | Tool / Package | Version |
| :--- | :--- | :--- |
| **Wasm SDK** | Emscripten (EMSDK) | **5.0.5** |
| **C Compilers** | GCC / G++ | 14 |
| **C++ Compilers** | Clang / Clang++ | 19.x & 20.x |
| **Build System** | CMake | Latest from apt |
| **Build System** | Ninja | Latest from apt |
| **Emulation** | QEMU | Latest from apt |
| **Documentation** | Doxygen | 1.16.1 |

### 📂 Environment Configuration

The container is initialized with these persistent environment variables:

```bash
EMSDK="/opt/wasm/emsdk"
EMSDK_VERSION="5.0.5"
EM_CONFIG="/opt/wasm/emsdk/.emscripten"
EMSDK_NODE="/opt/wasm/emsdk/node/22.16.0_64bit/bin/node"
LD_LIBRARY_PATH="/usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:..."
```

### 📦 Usage

#### Building the Image
```bash
docker build -t cpp-dev-env:latest .
```

#### Running the Container
```bash
docker run -it --rm -v $(pwd):/workspace -w /workspace cpp-dev-env:latest
```

#### Git Integration
The environment is pre-configured to handle GitHub Actions workspaces by setting `/github/workspace` as a safe directory.


## SYCL Version
This Dockerfile provides a lightweight, cutting-edge development environment based on **Arch Linux**. It is specifically tailored for **Intel oneAPI** development, enabling Data Parallel C++ (DPC++) and SYCL applications.

### 🚀 Key Features

* **Rolling Release Base:** Built on `archlinux/archlinux` for the latest stable packages.
* **Intel oneAPI Toolchain:** Includes the DPC++/C++ compiler and essential runtime libraries.
* **Modern Build Suite:** Equipped with CMake, Ninja, and Python for complex build pipelines.
* **Optimized Footprint:** Automated cleanup of `pacman` cache to keep image size minimal.

### 🛠 Software Inventory & Versions

Since this image is based on Arch Linux, packages generally reflect the **latest stable upstream versions** available at the time of the build.

#### Primary Toolchains
| Category | Tool / Package | Purpose |
| :--- | :--- | :--- |
| **Compiler** | `intel-oneapi-dpcpp-cpp` | Intel oneAPI DPC++/C++ Compiler (SYCL support) |
| **Runtime** | `intel-oneapi-compiler-dpcpp-cpp-runtime-libs` | Essential libraries for executing oneAPI binaries |
| **Base Compiler** | `gcc` | GNU Compiler Collection (System C/C++ support) |
| **Build System** | `cmake` | Cross-platform build automation |
| **Build System** | `ninja` | Small, high-speed build system |

#### Installed Packages Manifest
* **Development:** `intel-oneapi-dpcpp-cpp`, `intel-oneapi-compiler-dpcpp-cpp-runtime-libs`, `gcc`.
* **Build Tools:** `cmake`, `ninja`, `git`.
* **Scripting:** `python`.
* **Utilities:** `nano` (text editor).

### 📂 Configuration Details

* **User:** Operates as `root` for administrative flexibility.
* **Keyring:** `pacman-key --init` is executed during build to ensure secure package verification.
* **Maintenance:** The image performs a full system upgrade (`-Syu`) during build to ensure all system dependencies are current.

### 📦 Usage

#### Building the Image
```bash
docker build -t arch-oneapi:latest .
```

#### Running the Container
To start an interactive session with access to your local source code:
```bash
docker run -it --rm -v $(pwd):/projects -w /projects arch-oneapi:latest
```

#### Verifying the Intel Compiler
Once inside the container, you can verify the Intel environment:
```bash
icpx --version
```

### 📝 Notes
* **Storage:** The `pacman` cache is cleared (`/var/cache/pacman/pkg/`) during the build process to maintain a slim image.
* **Environment Variables:** Depending on your specific oneAPI workflow, you may need to source the Intel environment variables (e.g., `source /opt/intel/oneapi/setvars.sh`) if your application requires specific library paths at runtime.
