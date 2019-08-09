#!/bin/bash

. /etc/os-release

if [ "$ID" = "fedora" ]; then
  packages=(
    bison
    cmake
    flex
    gcc-c++
    llvm-devel
    make
  )
  dnf install -y ${packages[@]}
else
  echo "$ID is not supported"
fi
