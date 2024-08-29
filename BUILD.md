# OpenOMF Installation

## Compiling from source

Compiling from source should be your last option, unless you know what you are doing!

### Dependencies

Use at least GCC 9 or Clang 10. MSVC is not supported at this time.

Required:
* SDL2 (>=2.0.16): http://www.libsdl.org/download-2.0.php
* confuse: http://www.nongnu.org/confuse/
* Enet: http://enet.bespin.org/
* libargtable2 or libargtable3: http://argtable.sourceforge.net/ or http://www.argtable.org/
* libpng: http://www.libpng.org/pub/png/libpng.html
* zlib: http://www.zlib.net/ (for libpng)
* SDL2_Mixer (>=2.0.4): https://github.com/libsdl-org/SDL_mixer
* libxmp: https://github.com/cmatsuoka/libxmp


On Ubuntu, it is possible to pull some libraries using apt-get.
```
apt-get install libsdl2-dev libsdl2-mixer-dev libpng-dev libconfuse-dev libenet-dev libargtable2-dev libxmp-dev
```

On Mac, you can use brew:
```
brew install argtable sdl2_mixer confuse enet sdl2 libxmp libpng
```

### Acquiring the sources

Latest OpenOMF source can be acquired with the "git clone" command below.

```
git clone https://github.com/omf2097/openomf.git
```

Note that the latest sources do not necessarily compile, or they may have bugs. To retrieve
the latest source that compiles but will be out of date, see the
[releases](https://github.com/omf2097/openomf/releases) section of OpenOMF project in GitHub.

### Compiling

To compile (change install prefix if necessary):

```
$ mkdir -p build
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
$ make
$ make install
```

Some useful CMake flags:

| Flag                 | Description                             | Choices         | Default |
|----------------------|-----------------------------------------|-----------------|---------|
| CMAKE_BUILD_TYPE     | Chooses between build types             | -/Debug/Release | -       |
| CMAKE_INSTALL_PREFIX | Installation path                       | -               | -       |
| USE_TESTS            | Enables unittests (dev only!)           | On/Off          | Off     |
| USE_TOOLS            | Enables format editor tools (dev only!) | On/Off          | Off     |
| USE_SANITIZERS       | Enables asan and ubsan (dev only!)      | On/Off          | Off     |
| USE_FORMAT           | Enables clang-format (dev only!)        | On/Off          | Off     |
| USE_TIDY             | Enables clang-tidy (dev only!)          | On/Off          | Off     |

Note that when USE_FORMAT is selected, you can run command "make clangformat" to run code
formatter to the entire codebase.

## Data Files

OpenOMF loads the original data files from the original OMF:2097 game.
Since One Must Fall 2079 is freeware, the files are obtainable for free from
[www.omf2097.com](http://www.omf2097.com/pub/files/omf/omf2097.rar).

On linux in debug mode, the OMF resource files should be put in resources/ subdirectory.

On linux in release mode, the resources should be located in ../share/games/openomf/,
relative to the openomf binary. So if your binary is in /usr/local/bin, the resources should
be put in /usr/local/share/games/openomf/. With release or testing packages,
the resources should be extracted to /usr/share/games/openomf/.

On macOS and windows, the resources should be put into the resources/ subdirectory.

Note! Only BK,AF,PIC,TRN,PSM and DAT files are required, others may be removed to save space.

You can override these default paths by setting the OPENOMF_RESOURCE_DIR environment
variable to an absolute directory. Plugins have a similar environment variable called
OPENOMF_PLUGIN_DIR.

## Play the game!

Start the game by running the main binary. If there are problems, please contact us by either
making an issue ticket, or at #omf @ freenode in IRC (mode details in the
[README.md](https://github.com/omf2097/openomf/blob/master/README.md)).
