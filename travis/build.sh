#!/bin/sh

mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_OGGVORBIS=On -DTRAVIS=On ..
make && make test && make clean

cmake -DCMAKE_BUILD_TYPE=Release -DUSE_OGGVORBIS=On -DTRAVIS=On ..
make && make test
