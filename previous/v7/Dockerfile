##==================================================================================================
##  Recipe for ghcr.io/jfalcou/compilers:v7
##
##  The only recipe in this directory that is the original source rather than a reconstruction.
##  Identified by matching its ENV against every published tag: EMSDK_VERSION 3.1.49, node
##  16.20.0_64bit, INTEL_SDE_URL and DOXYGEN_URL - a combination unique to v7.
##
##  It is a record, not a build you should run. See previous/README.md.
##==================================================================================================
FROM ubuntu:jammy

ENV LD_LIBRARY_PATH /usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH            /opt/wasm/emsdk:/opt/wasm/emsdk/upstream/emscripten:/opt/wasm/emsdk/node/16.20.0_64bit/bin:/usr/local/bin:/opt/sde:$PATH
ENV DEBIAN_FRONTEND noninteractive
ENV INTEL_SDE_URL   https://www.intel.com/content/dam/develop/external/us/en/documents/downloads/sde-external-8.69.1-2021-07-18-lin.tar.bz2
ENV BOOST_URL       https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.gz
ENV DOXYGEN_URL     https://github.com/doxygen/doxygen/releases/download/Release_1_9_6/doxygen-1.9.6.linux.bin.tar.gz
ENV DOXYGEN_VERSION doxygen-1.9.6
ENV EMSDK           /opt/wasm/emsdk
ENV EMSDK_VERSION   3.1.49
ENV EM_CONFIG       /opt/wasm/emsdk/.emscripten
ENV EMSDK_NODE      /opt/wasm/emsdk/node/16.20.0_64bit/bin/node

RUN   apt-get update -y && apt-get install -y --no-install-recommends gpg-agent debian-keyring              \
      software-properties-common apt-utils                                                            &&    \
      add-apt-repository ppa:ubuntu-toolchain-r/test                                                  &&    \
      apt-get update -y                                                                               &&    \
      apt-get -y install sudo tzdata openssh-server curl wget libssl-dev libffi-dev ca-certificates   &&    \
      wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -                         &&    \
      add-apt-repository deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-15 main                  &&    \
      apt-get update -y

RUN   apt-get install -y --no-install-recommends                                                            \
      less unzip tar gzip zip tar                                                                           \
      python3 python3-defusedxml python3-lxml                                                               \
      build-essential ninja-build cmake git                                                                 \
      valgrind  jq  gdb lcov                                                                                \
      nano vim

RUN   apt-get install -y --no-install-recommends                                                            \
      qemu qemu-user qemu-user-binfmt libc6-arm64-cross

RUN   apt-get install -y --no-install-recommends                                                            \
      g++-12 gcc-12-multilib g++-12-multilib                                                                \
      g++-12-aarch64-linux-gnu g++-12-arm-linux-gnueabihf                                                   \
      g++-12-powerpc64-linux-gnu                                                                            \
      g++-12-powerpc64le-linux-gnu  g++-12-powerpc-linux-gnu                                                \
      binutils-aarch64-linux-gnu                                                                            \
      binutils-powerpc64-linux-gnu

RUN   apt-get install -y --no-install-recommends                                                            \
      libc++-15-dev libc++abi-15-dev clang-15 clang-format lld

RUN   apt-get install -y --no-install-recommends libopenmpi-dev libopenmpi3

RUN   ln -sf /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1     /lib/ld-linux-aarch64.so.1          &&    \
      ln -sf /usr/arm-linux-gnueabihf/lib/ld-linux-armhf.so.3     /lib/ld-linux-armhf.so.3            &&    \
      ln -sf /usr/powerpc64le-linux-gnu/lib64/ld64.so.2           /lib64/ld64.so.2                    &&    \
      ln -sf /usr/powerpc64-linux-gnu/lib64/ld64.so.1             /lib64/ld64.so.1

RUN   mkdir install && cd install                                                                     &&    \
      wget ${BOOST_URL}                                                                               &&    \
      tar -zxvf boost_1_87_0.tar.gz  && cd boost_1_87_0                                               &&    \
      ./bootstrap.sh --prefix=/usr/ && ./b2 && ./b2 install && cd ..

RUN   mkdir /opt/wasm && cd /opt/wasm && git clone https://github.com/emscripten-core/emsdk.git && cd emsdk \
      && git pull origin ${EMSDK_VERSION} && ./emsdk install latest  && ./emsdk activate latest  && cd ..

RUN   cd install &&     wget ${DOXYGEN_URL}                                                           &&    \
      tar -zxvf ${DOXYGEN_VERSION}.linux.bin.tar.gz && cp ${DOXYGEN_VERSION}/bin/* /usr/bin/

RUN   apt-get install -y --no-install-recommends llvm-15

RUN   apt clean && rm -rf /var/lib/apt/lists/*
RUN   git config --global --add safe.directory /github/workspace
RUN   ln -s /usr/bin/clang++-15 /usr/bin/clang++ && ln -s /usr/bin/clang-15 /usr/bin/clang
RUN   ln -s /usr/bin/llvm-cov-15 /usr/bin/llvm-cov && ln -s /usr/bin/llvm-profdata-15 /usr/bin/llvm-profdata