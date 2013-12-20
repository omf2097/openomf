#!/bin/sh

mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_OGGVORBIS=On -DTRAVIS=On ..
make && testing/test_main && make clean

cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DUSE_OGGVORBIS=On -DTRAVIS=On ..
make && testing/test_main
