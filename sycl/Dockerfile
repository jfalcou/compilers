FROM archlinux/archlinux

USER root

RUN pacman-key --init
RUN pacman -Syu --noconfirm --needed ninja python nano gcc cmake vim
RUN pacman -Syu --noconfirm --needed git intel-oneapi-compiler-dpcpp-cpp-runtime-libs intel-oneapi-dpcpp-cpp
RUN rm -rf /var/cache/pacman/pkg/
