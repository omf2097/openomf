#!/bin/sh

cd travis/
git clone https://github.com/omf2097/libShadowDive.git
git clone https://github.com/omf2097/dumb.git

cd libShadowDive
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install

cd ../../dumb
git checkout ttv-cmake
mkdir dumb/cmake/build/
cd dumb/cmake/build/
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install




