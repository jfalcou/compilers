FROM ubuntu:latest

ENV DEBIAN_FRONTEND noninteractive
ENV LD_LIBRARY_PATH /usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH            /opt/wasm/emsdk:/opt/wasm/emsdk/upstream/emscripten:/opt/wasm/emsdk/node/20.18.0_64bit/bin:/usr/local/bin:/opt/sde:$PATH

## BOOST CONFIG
ENV BOOST_URL       https://archives.boost.io/release/1.87.0/source/boost_1_87_0.tar.gz
ENV BOOST_TAG       boost_1_87_0

## EMSCRIPTEN CONFIG
ENV EMSDK           /opt/wasm/emsdk
ENV EMSDK_VERSION   4.0.5
ENV EM_CONFIG       /opt/wasm/emsdk/.emscripten
ENV EMSDK_NODE      /opt/wasm/emsdk/node/20.18.0_64bit/bin/node

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
      less unzip tar gzip zip tar python3 build-essential ninja-build cmake git valgrind gdb nano vim lcov

## G++ COMPILERS
RUN   apt-get install -y --no-install-recommends                                                                        \
      g++-14 gcc-14-multilib g++-14-multilib                                                                            \
      g++-14-aarch64-linux-gnu g++-14-arm-linux-gnueabihf                                                               \
      g++-14-powerpc64-linux-gnu  g++-14-powerpc64le-linux-gnu                                                          \
      g++-14-riscv64-linux-gnu

## CLANG AND LLVM TOOLS
RUN   apt-get install -y --no-install-recommends libc++-19-dev libc++abi-19-dev clang-19 clang-format lld llvm      &&  \
      ln -s /usr/bin/clang++-19 /usr/bin/clang++ && ln -s /usr/bin/clang-19 /usr/bin/clang

## MPI
RUN   apt-get install -y --no-install-recommends libopenmpi-dev libopenmpi3

## SETUP BOOST
RUN   cd install && wget -nv ${BOOST_URL}                                                                           &&  \
      tar -zxf ${BOOST_TAG}.tar.gz  && cd ${BOOST_TAG}                                                              &&  \
      ./bootstrap.sh --prefix=/usr/ && ./b2 && ./b2 install                                                         &&  \
      cd ..

## QEMU
RUN   apt-get install -y --no-install-recommends qemu-user qemu-user-binfmt libc6-arm64-cross

## DOXYGEN
RUN   apt-get install -y --no-install-recommends doxygen

## PROPAGATE CROSS-ARCH LIBSTD/C
RUN   ln -sf /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1     /lib/ld-linux-aarch64.so.1                        &&  \
      ln -sf /usr/arm-linux-gnueabihf/lib/ld-linux-armhf.so.3     /lib/ld-linux-armhf.so.3                          &&  \
      ln -sf /usr/powerpc64le-linux-gnu/lib64/ld64.so.2           /lib64/ld64.so.2                                  &&  \
      ln -sf /usr/powerpc64-linux-gnu/lib64/ld64.so.1             /lib64/ld64.so.1

## EMSCRIPTEN
RUN   mkdir /opt/wasm && cd /opt/wasm                                                                               &&  \
      git clone https://github.com/emscripten-core/emsdk.git && cd emsdk                                            &&  \
      git pull origin ${EMSDK_VERSION}                                                                              &&  \
      ./emsdk install latest  && ./emsdk activate latest                                                            &&  \
      cd ..

## SETUP INTEL SDE
RUN   cd install && curl https://downloadmirror.intel.com/823664/sde-external-9.38.0-2024-04-18-lin.tar.xz -o sde.tar.xz      &&  \
      tar xf sde.tar.xz                                                                                                       &&  \
      cp -r sde-external-9.38.0-2024-04-18-lin /opt/sde && \
      cd ..

## MAKE SAFE FOR GIT
RUN    git config --global --add safe.directory /github/workspace

## CLEANUP
RUN   apt clean && rm -rf /var/lib/apt/lists/* && rm -rf install