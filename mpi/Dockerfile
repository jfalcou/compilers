##======================================================================================================================
##  MPI Development Environment
##
##  The basic image plus OpenMPI. It lives apart because OpenMPI depends on gfortran, which drags a second GCC and the
##  whole ROCm chain through libucx0: 375 MB that no CI job of the fleet uses.
##======================================================================================================================
# syntax=docker/dockerfile:1.7

## The workflow passes no build-arg, so this default is the base that gets published. It moves to v11 in the same
## commit that publishes v11, since a pull request cannot build against a tag that does not exist yet.
ARG BASE_TAG=v10

FROM ghcr.io/jfalcou/compilers:${BASE_TAG}

ENV DEBIAN_FRONTEND=noninteractive

## LINK THE PACKAGE BACK TO ITS SOURCES SO GHCR GRANTS THIS REPO WRITE ACCESS
LABEL org.opencontainers.image.source="https://github.com/jfalcou/compilers"

RUN   apt-get update -y                                                                                             &&  \
      apt-get install -y --no-install-recommends libopenmpi-dev libopenmpi40                                        &&  \
      apt clean && rm -rf /var/lib/apt/lists/*

## SMOKE TEST
## Fails the build rather than shipping a toolchain that cannot run. Each tool is run unpiped once: a piped head would
## return head's exit status and pass over a missing binary.
RUN   mpicc  --showme:version > /dev/null                                                                           &&  \
      mpicxx --showme:version > /dev/null                                                                           &&  \
      mpirun --version        > /dev/null                                                                            &&  \
      mpirun --version | head -1
