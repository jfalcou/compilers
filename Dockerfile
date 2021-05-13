FROM ubuntu:devel

ENV LD_LIBRARY_PATH /usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH            $PATH:/usr/local/bin:/opt/sde
ENV DEBIAN_FRONTEND noninteractive
ENV INTEL_SDE_URL   https://software.intel.com/content/dam/develop/external/us/en/documents/downloads/sde-external-8.63.0-2021-01-18-lin.tar.bz2
ENV BOOST_URL       https://boostorg.jfrog.io/artifactory/main/release/1.75.0/source/boost_1_75_0.tar.gz

RUN   apt-get update -y && apt-get install -y --no-install-recommends gpg-agent debian-keyring              \
      software-properties-common apt-utils                                                            &&    \
      add-apt-repository ppa:ubuntu-toolchain-r/test                                                  &&    \
      apt-get update -y                                                                               &&    \
      apt-get -y install sudo tzdata openssh-server curl wget libssl-dev libffi-dev ca-certificates   &&    \
      wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -                         &&    \
      add-apt-repository deb http://apt.llvm.org/hirsute/ llvm-toolchain-hirsute main                 &&    \
      apt-get update -y                                                                               &&    \
      apt-get install -y --no-install-recommends                                                            \
      unzip tar gzip                                                                                        \
      python3 python3-defusedxml python3-lxml                                                               \
      build-essential ninja-build cmake git                                                                 \
      valgrind  jq  gdb                                                                                     \
      nano vim  clang-format-13                                                                             \
      qemu qemu-user qemu-user-binfmt libc6-arm64-cross                                                     \
      g++-11                                                                                                \
      gcc-11-multilib g++-11-multilib                                                                       \
      gcc-10-multilib g++-10-multilib                                                                       \
      g++-10-aarch64-linux-gnu      g++-11-aarch64-linux-gnu                                                \
      g++-10-arm-linux-gnueabihf    g++-11-arm-linux-gnueabihf                                              \
      binutils-aarch64-linux-gnu                                                                            \
      g++-10-powerpc64-linux-gnu    g++-11-powerpc64-linux-gnu                                              \
      g++-10-powerpc64le-linux-gnu  g++-11-powerpc64le-linux-gnu                                            \
      g++-10-powerpc-linux-gnu      g++-11-powerpc-linux-gnu                                                \
      binutils-powerpc64-linux-gnu                                                                          \
      clang-12 lld                                                                                          \
      &&                                                                                                    \
      ln -sf /usr/aarch64-linux-gnu/lib/ld-linux-aarch64.so.1     /lib/ld-linux-aarch64.so.1          &&    \
      ln -sf /usr/arm-linux-gnueabi/libhf/ld-linux-armhf.so.3     /lib/ld-linux-armhf.so.3            &&    \
      ln -sf /usr/powerpc64le-linux-gnu/lib64/ld64.so.2           /lib64/ld64.so.2                    &&    \
      ln -sf /usr/powerpc64-linux-gnu/lib64/ld64.so.1             /lib64/ld64.so.1                    &&    \
      mkdir install && cd install                                                                     &&    \
      wget ${INTEL_SDE_URL}                                                                           &&    \
      tar xf sde-external-8.63.0-2021-01-18-lin.tar.bz2                                               &&    \
      cp -r sde-external-8.63.0-2021-01-18-lin /opt/sde                                               &&    \
      wget ${BOOST_URL}                                                                               &&    \
      tar -zxvf boost_1_75_0.tar.gz  && cd boost_1_75_0                                               &&    \
      ./bootstrap.sh --prefix=/usr/ && ./b2 && ./b2 install && cd ..
