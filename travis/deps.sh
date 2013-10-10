#!/bin/sh

cd travis/
git clone https://github.com/omf2097/libShadowDive.git
git clone https://github.com/omf2097/libdumb-kataja.git

cd libShadowDive
mkdir build
cd build
cmake ..
make
make install

cd ../../libdumb-kataja
make -f Makefile.linux
make install



