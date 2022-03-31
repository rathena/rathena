#!/bin/bash

TOOLCHAIN="$1"

mkdir build
cd build

if [ "$TOOLCHAIN" != "" ] ; then
    cmake -DFASTFLOAT_TEST=ON  .. -DCMAKE_TOOLCHAIN_FILE=/toolchains/"$TOOLCHAIN".cmake
else
    cmake -DFASTFLOAT_TEST=ON  ..
fi
make -j 2
if [ "$TOOLCHAIN" != "" ] ; then
  qemu-"$TOOLCHAIN" tests/basictest
else
  ctest --output-on-failure -R basictest
fi
