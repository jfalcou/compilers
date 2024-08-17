FROM ubuntu:jammy

ENV LD_LIBRARY_PATH /usr/aarch64-linux-gnu/lib64:/usr/aarch64-linux-gnu/lib:/usr/arm-linux-gnueabihf/lib:/usr/powerpc64le-linux-gnu/lib/
ENV PATH            /opt/wasm/emsdk:/opt/wasm/emsdk/upstream/emscripten:/opt/wasm/emsdk/node/16.20.0_64bit/bin:/usr/local/bin:/opt/sde:$PATH
ENV DEBIAN_FRONTEND noninteractive

RUN   apt-get update -y && apt-get install -y --no-install-recommends gpg-agent debian-keyring              \
      software-properties-common apt-utils                                                            &&    \
      add-apt-repository ppa:ubuntu-toolchain-r/test                                                  &&    \
      apt-get update -y                                                                               &&    \
      apt-get -y install sudo tzdata openssh-server curl wget libssl-dev libffi-dev ca-certificates   &&    \
      wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -                         &&    \
      add-apt-repository deb http://apt.llvm.org/jammy/ llvm-toolchain-jammy-15 main                  &&    \
      apt-get update -y

RUN   apt-get install -y --no-install-recommends                                                            \
      less unzip tar gzip zip tar build-essential ninja-build cmake git gdb nano vim

RUN   apt-get install -y --no-install-recommends g++-12

RUN   apt-get install -y --no-install-recommends                                                            \
      libc++-15-dev libc++abi-15-dev clang-15 clang-format lld

RUN   git clone https://github.com/jfalcou/eve.git && \
      cd eve && \
      mkdir build && \
      cd build && \
      cmake .. && \
      make install

RUN   apt clean && rm -rf /var/lib/apt/lists/*
RUN   ln -s /usr/bin/clang++-15 /usr/bin/clang++ && ln -s /usr/bin/clang-15 /usr/bin/clang

RUN useradd --user-group --create-home --no-log-init --shell /bin/bash grayscott
USER grayscott
WORKDIR /home/grayscott
