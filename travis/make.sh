cd $TRAVIS_BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/bin/gcc-7 -DUSE_TESTS=On . 
make
make test
make clean
build-wrapper-linux-x86-64 --out-dir bw-output make
sonar-scanner
