#!/bin/sh

# remove the 2 lines below and add libsdl2-dev to apt-get if travis updates sdl to 2.0.1+
#wget http://libsdl.org/release/SDL2-2.0.3.tar.gz -O - | tar xz
#cd SDL2-2.0.3 && ./configure --prefix=$PREFIX && make -j5 && sudo make install
  
cd travis/
git clone --depth=1 https://github.com/omf2097/libShadowDive.git
git clone --depth=1 https://github.com/kode54/dumb.git

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




