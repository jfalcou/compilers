FROM ubuntu:latest

ENV LD_LIBRARY_PATH /usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH            $PATH:/usr/local/bin

RUN   apt-get update -y && apt-get install -y --no-install-recommends software-properties-common &&   \
      add-apt-repository ppa:ubuntu-toolchain-r/test &&                                               \
      apt-get update -y && apt-get install -y --no-install-recommends                                 \
      git openssh-server curl                                                                         \
      qemu qemu-user qemu-user-binfmt libc6-arm64-cross                                               \
      gcc g++ g++-10 clang lld                                                                        \
      gcc-10-multilib g++-10-multilib                                                                 \
      build-essential cmake ninja-build nano                                                          \
      g++-10-aarch64-linux-gnu                                                                        \
      g++-10-arm-linux-gnueabihf                                                                      \
      libstdc++-10-dev-arm64-cross                                                                    \
      binutils-aarch64-linux-gnu                                                                      \
      g++-10-powerpc64-linux-gnu                                                                      \
      g++-10-powerpc64le-linux-gnu                                                                    \
      g++-10-powerpc-linux-gnu                                                                        \
      binutils-powerpc64-linux-gnu                                                                    \
      unzip tar gzip sudo                                                                             \
      python3 python3-defusedxml python3-lxml                                                         \
      libssl-dev libffi-dev ca-certificates wget &&                                                   \
      ln -sf /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1 /lib/ld-linux-aarch64.so.1        &&    \
      ln -sf /usr/arm-linux-gnueabi/libhf/ld-linux-armhf.so.3 /lib/ld-linux-armhf.so.3          &&    \
      ln -sf /usr/powerpc64le-linux-gnu/lib64/ld64.so.2 /lib64/ld64.so.2                        &&    \
      mkdir install && cd install                                                               &&    \
      wget https://github.com/Kitware/CMake/releases/download/v3.19.3/cmake-3.19.3.tar.gz       &&    \
      tar -zxvf cmake-3.19.3.tar.gz  && cd cmake-3.19.3 && ./bootstrap                          &&    \
      make && sudo make install
