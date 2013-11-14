#!/bin/sh

mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_OGGVORBIS=On ..
