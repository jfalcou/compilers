FROM ubuntu:questing

ENV DEBIAN_FRONTEND=noninteractive
ENV LD_LIBRARY_PATH=/usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH=/opt/wasm/emsdk:/opt/wasm/emsdk/upstream/emscripten:/opt/wasm/emsdk/node/20.18.0_64bit/bin:/usr/local/bin:/opt/sde:$PATH

## EMSCRIPTEN CONFIG
ENV EMSDK=/opt/wasm/emsdk
ENV EMSDK_VERSION=4.0.5
ENV EM_CONFIG=/opt/wasm/emsdk/.emscripten
ENV EMSDK_NODE=/opt/wasm/emsdk/node/20.18.0_64bit/bin/node

## INSTALL BASE TOOLS
RUN   apt-get update -y                                                                                             &&  \
      apt-get install -y --no-install-recommends                                                                        \
      sudo tzdata openssh-server curl wget libssl-dev libffi-dev ca-certificates                                        \
      less unzip tar build-essential ninja-build cmake git valgrind gdb nano lcov                                   &&  \
      apt-get install -y --no-install-recommends g++-14 gcc-14-multilib g++-14-multilib                                 \
      g++-14-aarch64-linux-gnu g++-14-arm-linux-gnueabihf                                                               \
      g++-14-powerpc64-linux-gnu  g++-14-powerpc64le-linux-gnu                                                          \
      g++-14-riscv64-linux-gnu                                                                                      &&  \
      apt-get install -y --no-install-recommends libc++-19-dev libc++abi-19-dev clang-19 clang-format lld llvm      &&  \
      apt-get install -y --no-install-recommends libc++-20-dev libc++abi-20-dev clang-20                            &&  \
      apt-get install -y --no-install-recommends libclang-rt-19-dev libclang-rt-20-dev                              &&  \
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

## SETUP OTHER TOOLS
RUN   mkdir install && cd install                                                                                   &&  \
      curl https://downloadmirror.intel.com/823664/sde-external-9.38.0-2024-04-18-lin.tar.xz -o sde.tar.xz          &&  \
      tar xf sde.tar.xz                                                                                             &&  \
      cp -r sde-external-9.38.0-2024-04-18-lin /opt/sde                                                             &&  \
      cd .. && rm -rf install                                                                                       &&  \
      mkdir /opt/wasm && cd /opt/wasm                                                                               &&  \
      git clone https://github.com/emscripten-core/emsdk.git && cd emsdk                                            &&  \
      git pull origin ${EMSDK_VERSION}                                                                              &&  \
      ./emsdk install latest  && ./emsdk activate latest                                                            &&  \
      wget https://github.com/doxygen/doxygen/releases/download/Release_1_16_1/doxygen-1.16.1.linux.bin.tar.gz      &&  \
      tar xzf doxygen-1.16.1.linux.bin.tar.gz                                                                       &&  \
      cp doxygen-1.16.1/bin/doxygen /usr/local/bin/                                                                 &&  \
      rm -rf doxygen-1.16.1*