# OpenOMF Installation

## Installation from packages

Latest experimental binaries for windows, macOS and ubuntu are available at
https://github.com/omf2097/openomf/releases/tag/latest . These binaries may
not always work correctly, but they will be following the git status very 
closely. So, if you want to test the latest features, this is for you.

Official releases are (somewhat) stable, but don't necessary come with all 
the latest bells and whistles. However, if you wish to try them, they will
be available at https://github.com/omf2097/openomf/releases . Installation
instructions are also available there.

### Windows

1. Download an appropriate zip file. For 64bit computers, win64 package is available.
   If you do not know which package to download, get the "-win32.zip" version.
2. Extract the zip file somewhere on your computer. If possible, use a directory other
   than "C:\Program Files".
3. Install the game resources. Please see step 3. of this guide for this.
4. [optional] If you wish, you may install an xBRZ sprite scaler plugin.
   This can be found from https://github.com/omf2097/openomf-scaler-xbrz/releases .
   Note that this is not necessary for running the game.
5. Start the game by running openomf.exe.

### Ubuntu

1. Download an appropriate .deb file for your architecture (NOTE! 32bit not currently available!).
2. Install the package. You can either run dpkg -i <packagefile> or do it by using graphical tools
   in your distribution. Note that currently the packages are only tested on debian; Ubuntu may or
   may not work.
3. Install the game resources. Please see step 3. of this guide for this.
4. Start the game by running ./openomf on command line.

## Compiling from source

Compiling from source should be your last option, unless you know what you are doing!

### Dependencies

Use at least GCC 9 or Clang 10. MSVC is not supported at this time.

Required:
* SDL2 (>=2.0.5): http://www.libsdl.org/download-2.0.php
* confuse: http://www.nongnu.org/confuse/
* Gettext (if you have problems with libintl)
* Enet: http://enet.bespin.org/
* libargtable2: http://argtable.sourceforge.net/
* libpng: http://www.libpng.org/pub/png/libpng.html (for .PNG screenshots)
* zlib: http://www.zlib.net/ (for libpng)

Recommended:
* OpenAL: http://kcat.strangesoft.net/openal.html (for audio)
* libxmp: https://github.com/cmatsuoka/libxmp (for module music)

Optional:
* libvorbis: https://www.xiph.org/downloads/
* libogg: https://www.xiph.org/downloads/
* libdumb (>=2.0.0): https://bitbucket.org/kode54/dumb (for module music)


On debian, it is possible to pull some libraries using apt-get.
```
apt-get install libsdl2-dev libopenal-dev libpng-dev libconfuse-dev libenet-dev libargtable2-dev libxmp-dev
```

On Mac, you can use brew:
```
brew install argtable openal-soft confuse enet sdl2 libxmp libpng
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
| USE_OGGVORBIS        | Selects Vorbis support                  | On/Off          | Off     |
| USE_OPENAL           | Selects OpenAL support                  | On/Off          | On      |
| USE_DUMB             | Selects libdumb support                 | On/Off          | Off     |
| USE_XMP              | Selects libxmp support                  | On/Off          | On      |
| USE_TESTS            | Enables unittests (dev only!)           | On/Off          | Off     |
| USE_TOOLS            | Enables format editor tools (dev only!) | On/Off          | Off     |
| USE_SANITIZERS       | Enables asan and ubsan (dev only!)      | On/Off          | Off     |
| USE_FORMAT           | Enables clang-format (dev only!)        | On/Off          | Off     |
| USE_TIDY             | Enables clang-tidy (dev only!)          | On/Off          | Off     |

Ogg Vorbis support is required if you wish to replace original OMF soundtracks with OGG files.
Otherwise, the switch is optional.

For music playback, select at least one (or more) of the module player libraries. 
XMP is recommended due to ease of installation, but DUMB will also work just fine.

It is technically possible to select more than one audio sink, or none. Currently, only one
audio sink is supported (OpenAL). If all audio sinks are off, then no audio will be played.
This will also of course reduce cpu usage a bit.

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
