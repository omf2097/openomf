#!/bin/sh

mkdir build
cd build

cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_SUBMODULES=Off -DUSE_OGGVORBIS=On -DUSE_PNG=On -DTRAVIS=On ..
make && make test && make clean

cmake -DCMAKE_BUILD_TYPE=Release -DUSE_SUBMODULES=Off -DUSE_OGGVORBIS=On -DUSE_PNG=On -DTRAVIS=On ..
make && make test
