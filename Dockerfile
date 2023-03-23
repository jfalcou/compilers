FROM ubuntu:jammy

ENV LD_LIBRARY_PATH /usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH            /install/emsdk:/install/emsdk/upstream/emscripten:/install/emsdk/node/14.18.2_64bit/bin:/usr/local/bin:/opt/sde:$PATH
ENV DEBIAN_FRONTEND noninteractive
ENV INTEL_SDE_URL   https://www.intel.com/content/dam/develop/external/us/en/documents/downloads/sde-external-8.69.1-2021-07-18-lin.tar.bz2
ENV BOOST_URL       https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz
ENV DOXYGEN_URL     https://github.com/doxygen/doxygen/releases/download/Release_1_9_6/doxygen-1.9.6.linux.bin.tar.gz
ENV EMSDK           /install/emsdk
ENV EM_CONFIG       /install/emsdk/.emscripten
ENV EMSDK_NODE      /install/emsdk/node/14.18.2_64bit/bin/node

RUN   apt-get update -y && apt-get install -y --no-install-recommends gpg-agent debian-keyring              \
      software-properties-common apt-utils                                                            &&    \
      add-apt-repository ppa:ubuntu-toolchain-r/test                                                  &&    \
      apt-get update -y                                                                               &&    \
      apt-get -y install sudo tzdata openssh-server curl wget libssl-dev libffi-dev ca-certificates   &&    \
      wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -                         &&    \
      add-apt-repository deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-14 main                  &&    \
      apt-get update -y

RUN   apt-get install -y --no-install-recommends                                                            \
      less unzip tar gzip                                                                                   \
      python3 python3-defusedxml python3-lxml                                                               \
      build-essential ninja-build cmake git                                                                 \
      valgrind  jq  gdb                                                                                     \
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
      libc++-14-dev libc++abi-14-dev clang clang-format lld libclang-rt-14-dev

RUN   apt-get install -y --no-install-recommends libopenmpi-dev libopenmpi3

RUN   ln -sf /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1     /lib/ld-linux-aarch64.so.1          &&    \
      ln -sf /usr/arm-linux-gnueabihf/lib/ld-linux-armhf.so.3     /lib/ld-linux-armhf.so.3            &&    \
      ln -sf /usr/powerpc64le-linux-gnu/lib64/ld64.so.2           /lib64/ld64.so.2                    &&    \
      ln -sf /usr/powerpc64-linux-gnu/lib64/ld64.so.1             /lib64/ld64.so.1

RUN   mkdir install && cd install                                                                     &&    \
      curl ${INTEL_SDE_URL} --output sde.tar.bz2                                                      &&    \
      tar xf sde.tar.bz2                                                                              &&    \
      cp -r sde-external-8.69.1-2021-07-18-lin /opt/sde

RUN   cd install &&     wget ${BOOST_URL}                                                             &&    \
      tar -zxvf boost_1_80_0.tar.gz  && cd boost_1_80_0                                               &&    \
      ./bootstrap.sh --prefix=/usr/ && ./b2 && ./b2 install && cd ..

RUN   cd install && git clone https://github.com/emscripten-core/emsdk.git && cd emsdk                &&    \
      git pull && ./emsdk install latest  && ./emsdk activate latest  && cd ..

RUN   cd install &&     wget ${DOXYGEN_URL}                                                             &&    \
      tar -zxvf doxygen-1.9.6.linux.bin.tar.gz && cp doxygen-1.9.6/bin/* /usr/bin/

RUN   apt clean && rm -rf install/* && rm -rf /var/lib/apt/lists/*
RUN   git config --global --add safe.directory /github/workspace
