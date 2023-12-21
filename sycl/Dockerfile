FROM archlinux/archlinux

USER root

RUN pacman-key --init

RUN pacman -Syu --noconfirm --needed                                                        \
    gcc nano cmake git intel-oneapi-compiler-dpcpp-cpp-runtime-libs intel-oneapi-dpcpp-cpp

RUN pacman -Syu --noconfirm --needed ninja python

RUN rm -rf /var/cache/pacman/pkg/
