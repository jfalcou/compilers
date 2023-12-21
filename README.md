# compilers

Cross compiling docker for C++20

Those dockerfiles provide :

* BASIC:
  - a Ubuntu Jammy base with :
    - g++-12
    - clang++-14
    - Emscripten
    - Cross compilers for ARM, AARCH64, POWERPC, POWERPC64 and POWREPC64LE
    - QEMU setup for ARM, AARCH64, POWERPC, POWERPC64 and POWREPC64LE
    - Intel SDE
    - Boost 1.80
    - python3
    - cmake
    - ninja
    - git
    - Doxygen 1.9.6
    - SPACK
    - Conan
    - VCPKG

* SYCL:
  - a ArchLinux base with :
    - g++
    - Intel DPC++
    - python3
    - cmake
    - ninja
    - git
