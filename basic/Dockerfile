##======================================================================================================================
##  Multi-Arch C++ & WebAssembly Development Environment
##
##  Four independent stages so BuildKit can fetch the tools while the toolchain installs, and so a change to one does
##  not invalidate the others. Everything the final image keeps arrives through COPY --link.
##======================================================================================================================
# syntax=docker/dockerfile:1.7

ARG EMSDK_VERSION=5.0.5
ARG DOXYGEN_VERSION=1.16.1
ARG DOXYGEN_SHA256=a56f885d37e3aae08a99f638d17bbb381224c03a878d9e2dda4f9fa4baf1d8bd

##======================================================================================================================
## COMPILERS AND CROSS-TOOLCHAINS - the long pole, ~5 minutes
##======================================================================================================================
FROM ubuntu:questing AS toolchains

ENV DEBIAN_FRONTEND=noninteractive

RUN   apt-get update -y                                                                                             &&  \
      apt-get install -y --no-install-recommends                                                                        \
      sudo tzdata openssh-server curl wget libssl-dev libffi-dev ca-certificates                                        \
      less unzip tar build-essential ninja-build cmake git valgrind gdb nano lcov "gcovr=7.*"                            \
      python3                                                                                                       &&  \
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

##======================================================================================================================
## EMSCRIPTEN SDK - built aside, only /opt/wasm reaches the final image
##======================================================================================================================
FROM ubuntu:questing AS emsdk

ARG EMSDK_VERSION
ENV DEBIAN_FRONTEND=noninteractive

RUN   apt-get update -y                                                                                             &&  \
      apt-get install -y --no-install-recommends git python3 ca-certificates xz-utils                               &&  \
      rm -rf /var/lib/apt/lists/*

## Cloning the tag pins the SDK. Resolving node through a version-free symlink is what keeps ENV EMSDK_NODE valid:
## the SDK picks its own node version, and spelling one out is what broke every wasm job on 2026-08-26.
RUN   mkdir /opt/wasm && cd /opt/wasm                                                                               &&  \
      git clone --depth 1 --branch "${EMSDK_VERSION}" https://github.com/emscripten-core/emsdk.git                  &&  \
      cd emsdk                                                                                                      &&  \
      ./emsdk install "${EMSDK_VERSION}" && ./emsdk activate "${EMSDK_VERSION}"                                     &&  \
      ln -sfn "$(ls -d /opt/wasm/emsdk/node/*_64bit | head -1)" /opt/wasm/emsdk/node/current

##======================================================================================================================
## DOXYGEN - a single binary, fetched by pinned version and verified before it is unpacked
##======================================================================================================================
FROM ubuntu:questing AS doxygen

ARG DOXYGEN_VERSION
ARG DOXYGEN_SHA256
ENV DEBIAN_FRONTEND=noninteractive

RUN   apt-get update -y                                                                                             &&  \
      apt-get install -y --no-install-recommends wget ca-certificates                                               &&  \
      rm -rf /var/lib/apt/lists/*

RUN   DOXYGEN_TAG=Release_$(echo "${DOXYGEN_VERSION}" | tr . _)                                                     &&  \
      cd /tmp                                                                                                       &&  \
      wget -q "https://github.com/doxygen/doxygen/releases/download/${DOXYGEN_TAG}/doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz" && \
      echo "${DOXYGEN_SHA256}  doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz" | sha256sum -c -                        &&  \
      tar xzf "doxygen-${DOXYGEN_VERSION}.linux.bin.tar.gz"                                                         &&  \
      mkdir /out && cp "doxygen-${DOXYGEN_VERSION}/bin/doxygen" /out/doxygen

##======================================================================================================================
## THE IMAGE ITSELF
##======================================================================================================================
FROM toolchains AS final

ARG EMSDK_VERSION

## LINK THE PACKAGE BACK TO ITS SOURCES SO GHCR GRANTS THIS REPO WRITE ACCESS
LABEL org.opencontainers.image.source="https://github.com/jfalcou/compilers"

## MAKE SAFE FOR GIT
RUN    git config --global --add safe.directory /github/workspace

# SETUP PATH
ENV LD_LIBRARY_PATH=/usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH=/opt/wasm/emsdk:/opt/wasm/emsdk/upstream/emscripten:/opt/wasm/emsdk/node/current/bin:/usr/local/bin:$PATH

## EMSCRIPTEN CONFIG
ENV EMSDK=/opt/wasm/emsdk
ENV EMSDK_VERSION=${EMSDK_VERSION}
ENV EM_CONFIG=/opt/wasm/emsdk/.emscripten
ENV EMSDK_NODE=/opt/wasm/emsdk/node/current/bin/node

## --link makes each of these a layer of its own: changing the package list above no longer rebuilds them.
COPY --link --from=emsdk   /opt/wasm    /opt/wasm
COPY --link --from=doxygen /out/doxygen /usr/local/bin/doxygen

## SMOKE TEST
## Fails the build rather than shipping a toolchain that cannot run. It is the only place that sees the assembled
## image, so it stays here. Each tool is run unpiped once: a piped head would swallow its exit status.
RUN   node --version                                                                                                &&  \
      emcc --version > /dev/null                                                                                    &&  \
      emcc --version | head -1                                                                                      &&  \
      gcovr --version | head -1                                                                                     &&  \
      if ! gcovr --version | grep -q "^gcovr 7\."; then echo "FATAL: gcovr must be 7.x"; exit 1; fi
