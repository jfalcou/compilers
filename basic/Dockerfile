FROM ubuntu:questing

ENV DEBIAN_FRONTEND=noninteractive

## LINK THE PACKAGE BACK TO ITS SOURCES SO GHCR GRANTS THIS REPO WRITE ACCESS
LABEL org.opencontainers.image.source="https://github.com/jfalcou/compilers"

## INSTALL BASE TOOLS
RUN   apt-get update -y                                                                                             &&  \
      apt-get install -y --no-install-recommends                                                                        \
      sudo tzdata openssh-server curl wget libssl-dev libffi-dev ca-certificates                                        \
      less unzip tar build-essential ninja-build cmake git valgrind gdb nano lcov "gcovr=7.*"                       &&  \
      apt-get install -y --no-install-recommends g++-14 gcc-14-multilib g++-14-multilib                                 \
      g++-14-aarch64-linux-gnu g++-14-arm-linux-gnueabihf                                                               \
      g++-14-riscv64-linux-gnu g++-14-powerpc64-linux-gnu  g++-14-powerpc64le-linux-gnu                             &&  \
      apt-get install -y --no-install-recommends libmpfrc++-dev libmpfr-dev libgmp-dev                              &&  \
      apt-get install -y --no-install-recommends libc++-19-dev libc++abi-19-dev clang-19 clang-format lld llvm      &&  \
      apt-get install -y --no-install-recommends libc++-20-dev libc++abi-20-dev clang-20                            &&  \
      apt-get install -y --no-install-recommends libc++-21-dev libc++abi-21-dev clang-21                            &&  \
      apt-get install -y --no-install-recommends libclang-rt-19-dev libclang-rt-20-dev libclang-rt-21-dev           &&  \
      apt-get install -y --no-install-recommends libopenmpi-dev libopenmpi40                                            \
      qemu-user qemu-user-binfmt libc6-arm64-cross libboost-dev                                                     &&  \
      apt clean && rm -rf /var/lib/apt/lists/*                                                                      &&  \
      ln -sf /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1     /lib/ld-linux-aarch64.so.1                        &&  \
      ln -sf /usr/arm-linux-gnueabihf/lib/ld-linux-armhf.so.3     /lib/ld-linux-armhf.so.3                          &&  \
      ln -sf /usr/powerpc64le-linux-gnu/lib64/ld64.so.2           /lib64/ld64.so.2                                  &&  \
      ln -sf /usr/powerpc64-linux-gnu/lib64/ld64.so.1             /lib64/ld64.so.1                                  &&  \
      ln -s /usr/bin/clang++-19                                   /usr/bin/clang++                                  &&  \
      ln -s /usr/bin/clang-19                                     /usr/bin/clang



## MAKE SAFE FOR GIT
RUN    git config --global --add safe.directory /github/workspace

# SETUP PATH
ENV LD_LIBRARY_PATH=/usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH=/opt/wasm/emsdk:/opt/wasm/emsdk/upstream/emscripten:/opt/wasm/emsdk/node/current/bin:/usr/local/bin:/opt/sde:$PATH

## EMSCRIPTEN CONFIG
ENV EMSDK=/opt/wasm/emsdk
ENV EMSDK_VERSION=5.0.5
ENV EM_CONFIG=/opt/wasm/emsdk/.emscripten
ENV EMSDK_NODE=/opt/wasm/emsdk/node/current/bin/node

## SETUP OTHER TOOLS
## Versions and checksums are pinned: a moved URL or a replaced tarball fails the build
## instead of quietly installing something else.
ARG SDE_VERSION=10.8.0-2026-03-15
ARG SDE_SHA256=50b320cd226acef7a491f5b321fc1be3c3c7984f9e27a456e64894b5b0979dd3
ARG DOXYGEN_VERSION=1.16.1
ARG DOXYGEN_SHA256=a56f885d37e3aae08a99f638d17bbb381224c03a878d9e2dda4f9fa4baf1d8bd

RUN   mkdir install && cd install                                                                                   &&  \
      SDE=sde-external-${SDE_VERSION}-lin                                                                           &&  \
      DOXYGEN_TAG=Release_$(echo "${DOXYGEN_VERSION}" | tr . _)                                                     &&  \
      curl -fsSL "https://downloadmirror.intel.com/915934/${SDE}.tar.xz" -o sde.tar.xz                              &&  \
      echo "${SDE_SHA256}  sde.tar.xz" | sha256sum -c -                                                             &&  \
      tar xf sde.tar.xz                                                                                             &&  \
      cp -r "${SDE}" /opt/sde                                                                                       &&  \
      cd .. && rm -rf install                                                                                       &&  \
      mkdir /opt/wasm && cd /opt/wasm                                                                               &&  \
      git clone --depth 1 --branch "${EMSDK_VERSION}" https://github.com/emscripten-core/emsdk.git                  &&  \
      cd emsdk                                                                                                      &&  \
      ./emsdk install "${EMSDK_VERSION}" && ./emsdk activate "${EMSDK_VERSION}"                                     &&  \
      ln -sfn "$(ls -d /opt/wasm/emsdk/node/*_64bit | head -1)" /opt/wasm/emsdk/node/current                        &&  \
      wget -q "https://github.com/doxygen/doxygen/releases/download/${DOXYGEN_TAG}/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz" &&  \
      echo "${DOXYGEN_SHA256}  doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz" | sha256sum -c -                        &&  \
      tar xzf "doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz"                                                         &&  \
      cp "doxygen-${DOXYGEN_VERSION}/bin/doxygen" /usr/local/bin/                                                   &&  \
      rm -rf "doxygen-${DOXYGEN_VERSION}"*

## SMOKE TEST
## Fails the build rather than shipping a toolchain that cannot run. The ENV above is applied
## here, so a PATH pointing at a node that is not there stops the image on the spot.
## gcovr is asserted, not just printed: only 7.x measures header coverage as TTS expects.
## Each tool is run unpiped once: a piped head would swallow its exit status.
RUN   node --version                                                                                                &&  \
      emcc --version > /dev/null                                                                                    &&  \
      emcc --version | head -1                                                                                      &&  \
      gcovr --version | head -1                                                                                     &&  \
      if ! gcovr --version | grep -q "^gcovr 7\."; then echo "FATAL: gcovr must be 7.x"; exit 1; fi
