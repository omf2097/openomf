#!/bin/sh

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DUSE_OGGVORBIS=On -DTRAVIS=On ..
make && find testing/ -type f -perm +111 -print0 | xargs -0 source && make clean
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_OGGVORBIS=On -DTRAVIS=On ..
make && find testing/ -type f -perm +111 -print0 | xargs -0 source
