FROM nvcr.io/nvidia/cuda:12.3.1-devel-ubi8

ENV DEBIAN_FRONTEND noninteractive

RUN   apt-get update -y && apt-get install -y --no-install-recommends gpg-agent debian-keyring              \
      software-properties-common apt-utils                                                            &&    \
      add-apt-repository ppa:ubuntu-toolchain-r/test                                                  &&    \
      apt-get update -y                                                                               &&    \
      apt-get -y install sudo tzdata openssh-server curl wget libssl-dev libffi-dev ca-certificates

RUN   apt-get install -y --no-install-recommends                                                            \
      less unzip tar gzip zip tar                                                                           \
      python3 python3-defusedxml python3-lxml                                                               \
      ninja-build cmake git                                                                                 \
      nano vim

RUN   apt clean && rm -rf /var/lib/apt/lists/*
RUN   git config --global --add safe.directory /github/workspace
RUN   ln -s /usr/bin/clang++-15 /usr/bin/clang++ && ln -s /usr/bin/clang-15 /usr/bin/clang
