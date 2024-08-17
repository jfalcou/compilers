FROM ubuntu:mantic

ENV DEBIAN_FRONTEND noninteractive
ENV LD_LIBRARY_PATH /usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH            /opt/wasm/emsdk:/opt/wasm/emsdk/upstream/emscripten:/opt/wasm/emsdk/node/16.20.0_64bit/bin:/usr/local/bin:/opt/sde:$PATH

## BOOST CONFIG
ENV BOOST_URL       https://boostorg.jfrog.io/artifactory/main/release/1.80.0/source/boost_1_80_0.tar.gz

## DOXYGEN CONFIG
ENV DOXYGEN_URL     https://github.com/doxygen/doxygen/releases/download/Release_1_9_6/doxygen-1.9.6.linux.bin.tar.gz
ENV DOXYGEN_VERSION doxygen-1.9.6

## EMSCRIPTEN CONFIG
ENV EMSDK           /opt/wasm/emsdk
ENV EMSDK_VERSION   3.1.64
ENV EM_CONFIG       /opt/wasm/emsdk/.emscripten
ENV EMSDK_NODE      /opt/wasm/emsdk/node/16.20.0_64bit/bin/node

## SETUP APT
RUN   mkdir install                                                                                                 &&  \
      apt-get update -y                                                                                             &&  \
      apt-get install -y --no-install-recommends                                                                        \
      gpg-agent debian-keyring software-properties-common apt-utils                                                 &&  \
      add-apt-repository ppa:ubuntu-toolchain-r/test

## INSTALL BASE TOOLS
RUN   apt-get update -y                                                                                             &&  \
      apt-get install -y                                                                                                \
      sudo tzdata openssh-server curl wget libssl-dev libffi-dev ca-certificates                                    &&  \
      apt-get install -y --no-install-recommends                                                                        \
      less unzip tar gzip zip tar python3 build-essential ninja-build cmake git valgrind gdb nano vim

## QEMU
RUN   apt-get install -y --no-install-recommends qemu-user qemu-user-binfmt libc6-arm64-cross

## G++ CROSS-COMPILERS
RUN   apt-get install -y --no-install-recommends                                                                        \
      gcc-13-multilib g++-13-multilib                                                                                   \
      gcc-13-aarch64-linux-gnu gcc-13-arm-linux-gnueabihf                                                               \
      g++-13-powerpc64-linux-gnu  g++-13-powerpc64le-linux-gnu

## CLANG AND LLVM TOOLS
RUN   wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | sudo tee /etc/apt/trusted.gpg.d/apt.llvm.org.asc       &&  \
      add-apt-repository deb http://apt.llvm.org/mantic/ llvm-toolchain-mantic-18 main                              &&  \
      apt-get update -y                                                                                             &&  \
      apt-get install -y --no-install-recommends libc++-18-dev libc++abi-18-dev clang-18 clang-format lld           &&  \
      ln -s /usr/bin/clang++-18 /usr/bin/clang++ && ln -s /usr/bin/clang-18 /usr/bin/clang

### MPI
RUN   apt-get install -y --no-install-recommends libopenmpi-dev libopenmpi3

## PROPAGATE CROSS-ARCH LIBSTD/C
RUN   ln -sf /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1     /lib/ld-linux-aarch64.so.1                        &&  \
      ln -sf /usr/arm-linux-gnueabihf/lib/ld-linux-armhf.so.3     /lib/ld-linux-armhf.so.3                          &&  \
      ln -sf /usr/powerpc64le-linux-gnu/lib64/ld64.so.2           /lib64/ld64.so.2                                  &&  \
      ln -sf /usr/powerpc64-linux-gnu/lib64/ld64.so.1             /lib64/ld64.so.1

## DOXYGEN
RUN   cd install && wget ${DOXYGEN_URL}                                                                             &&  \
      tar -zxvf ${DOXYGEN_VERSION}.linux.bin.tar.gz && cp ${DOXYGEN_VERSION}/bin/* /usr/bin/                        &&  \
      cd ..

## EMSCRIPTEN
RUN   mkdir /opt/wasm && cd /opt/wasm                                                                               &&  \
      git clone https://github.com/emscripten-core/emsdk.git && cd emsdk                                            &&  \
      git pull origin ${EMSDK_VERSION}                                                                              &&  \
      ./emsdk install latest  && ./emsdk activate latest                                                            &&  \
      cd ..

## SETUP BOOST
RUN   cd install && wget ${BOOST_URL}                                                                               &&  \
      tar -zxvf boost_1_80_0.tar.gz  && cd boost_1_80_0                                                             &&  \
      ./bootstrap.sh --prefix=/usr/ && ./b2 && ./b2 install                                                         &&  \
      cd ..

## MAKE SAFE FOR GIT
RUN    git config --global --add safe.directory /github/workspace

## CLEANUP
RUN   apt clean && rm -rf /var/lib/apt/lists/*                                                                      &&  \
      rm -rf install/boost_1_80_0.tar.gz                                                                            &&  \
      rm -rf install/boost_1_80_0/                                                                                  &&  \
      rm -rf install/${DOXYGEN_VERSION}.linux.bin.tar.gz                                                            &&  \
      rm -rf install/${DOXYGEN_VERSION}
