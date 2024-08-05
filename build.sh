#!/bin/sh

cd "$(dirname "$0")"

if [ ! -d ./build/ ]; then
  mkdir build
fi

cd build/

cmake -DCMAKE_CXX_COMPILER=clang++ ..

cmake --build .

../bin/Pali
