sudo add-apt-repository ppa:zoogie/sdl2-snapshots -y
sudo add-apt-repository ppa:george-edison55/cmake-3.x -y
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update
sudo apt-get -y install \
    gcc-7 \
    cmake \
    libargtable2-dev \
    libcunit1-dev \
    libopenal-dev \
    libconfuse-dev \
    libenet-dev \
    libsdl2-dev

sudo apt-get remove libpng12-dev libz-dev -y
wget "http://mirrors.kernel.org/ubuntu/pool/main/z/zlib/zlib1g-dev_1.2.11.dfsg-0ubuntu1_amd64.deb"
wget "http://mirrors.kernel.org/ubuntu/pool/main/z/zlib/zlib1g_1.2.11.dfsg-0ubuntu1_amd64.deb"
wget "http://mirrors.kernel.org/ubuntu/pool/main/libp/libpng1.6/libpng16-16_1.6.34-1_amd64.deb"
wget "http://mirrors.kernel.org/ubuntu/pool/main/libp/libpng1.6/libpng-tools_1.6.34-1_amd64.deb"
wget "http://mirrors.kernel.org/ubuntu/pool/main/libp/libpng1.6/libpng-dev_1.6.34-1_amd64.deb"
sudo dpkg -i zlib1g_1.2.11.dfsg-0ubuntu1_amd64.deb
sudo dpkg -i zlib1g-dev_1.2.11.dfsg-0ubuntu1_amd64.deb
sudo dpkg -i libpng16-16_1.6.34-1_amd64.deb
sudo dpkg -i libpng-tools_1.6.34-1_amd64.deb
sudo dpkg -i libpng-dev_1.6.34-1_amd64.deb
