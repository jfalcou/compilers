FROM ubuntu:questing

ENV DEBIAN_FRONTEND=noninteractive

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


RUN   git clone https://github.com/jfalcou/eve.git && \
      cd eve && \
      mkdir build && \
      cd build && \
      cmake .. && \
      make install

RUN useradd --user-group --create-home --no-log-init --shell /bin/bash grayscott
USER grayscott
WORKDIR /home/grayscott

COPY basic.cpp /home/grayscott/basic.cpp
COPY math.cpp /home/grayscott/math.cpp
COPY stencil.cpp /home/grayscott/stencil.cpp
COPY stencil2.cpp /home/grayscott/stencil2.cpp