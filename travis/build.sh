#!/bin/sh

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_OGGVORBIS=On ..
make
