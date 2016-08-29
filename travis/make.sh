if [ "${COVERITY_SCAN_BRANCH}" != 1 ]; then 
    cd $TRAVIS_BUILD_DIR
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=/usr/bin/gcc-6 -DUSE_TESTS=On . 
    make
    make test
fi