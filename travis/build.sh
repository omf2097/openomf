#!/bin/sh

cd build
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_OGGVORBIS=On ..
