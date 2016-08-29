sudo add-apt-repository ppa:zoogie/sdl2-snapshots -y
sudo add-apt-repository ppa:george-edison55/cmake-3.x -y
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update
sudo apt-get -y install \
    gcc-6 \
    cmake \
    libargtable2-dev \
    libcunit1-dev \
    libopenal-dev \
    libconfuse-dev \
    libpng-dev \
    libenet-dev \
    libsdl2-dev