#!/bin/sh

sudo apt-get --force-yes install libopenal-dev libglew-dev libsdl2-static-dev libconfuse-dev libenet-dev libogg-dev libvorbis-dev

cd travis/
git clone https://github.com/omf2097/libShadowDive.git
git clone https://bitbucket.org/kode54/dumb.git

cd libShadowDive
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install

cd ../../dumb
mkdir dumb/cmake/build/
cd dumb/cmake/build/
cmake -DCMAKE_BUILD_TYPE=Release ..
make
make install




