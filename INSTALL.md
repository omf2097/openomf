# OpenOMF Installation

Note! If you wish to compile OpenOMF from sources yourself, begin from step 2.
If you are however downloading a binary release package, you should skip step 2.

## 1. Installation from packages

### Latest experimental binaries

Latest experimental binaries for windows and debian (ubuntu) are available at
https://buildbot.openomf.org/artifacts/ . These binaries may not always work
correctly, but they will be following the git status very closely. So, if you
with to test the latest features, this is for you!

#### Windows

1. Download an appropriate zip file. For 64bit computers, win64 package is available.
   If you do not know which package to download, get the "-win32.zip" version.
2. Extract the zip file somewhere on your computer. If possible, use a directory other
   than "C:\Program Files".
3. Install the game resources. Please see step 3. of this guide for this.
4. [optional] If you wish, you may install an xBRZ sprite scaler plugin.
   This can be found from https://github.com/omf2097/openomf-scaler-xbrz/releases .
   Note that this is not necessary for running the game.
5. Start the game by running openomf.exe.

#### Debian

1. Download an appropriate .deb file for your architecture (NOTE! 32bit not currently available!).
2. Install the package. You can either run dpkg -i <packagefile> or do it by using graphical tools
   in your distribution. Note that currently the packages are only tested on debian; Ubuntu may or
   may not work.
3. Install the game resources. Please see step 3. of this guide for this.
4. Start the game by running ./openomf on command line.

### Official releases

Official releases are (somewhat) stable, but don't necessary come with all the latest bells and
whistles. However, if you wish to try them, they will be available at
https://github.com/omf2097/openomf/releases . Installation instructions are also available there.

## 2. Compiling from source

### Dependencies

#### Compiler

Use at least GCC 4.9 or Clang 4. MSVC is not supported at this time.

Note! For debug builds, GCC 4.9 or later is recommended. With later GCC versions you get better
error messages and stricted code quality checks, so if you're doing development, please use
the latest compiler version available to you.

#### Libraries

Required:
* libShadowDive: https://github.com/omf2097/libShadowDive
* SDL2 (>=2.0.5): http://www.libsdl.org/download-2.0.php
* confuse: http://www.nongnu.org/confuse/
* Gettext (if you have problems with libintl)
* Enet: http://enet.bespin.org/
* libargtable2: http://argtable.sourceforge.net/

Recommended:
* OpenAL: http://kcat.strangesoft.net/openal.html (for audio)
* libxmp: https://github.com/cmatsuoka/libxmp (for module music)

Optional:
* libvorbis: https://www.xiph.org/downloads/
* libogg: https://www.xiph.org/downloads/
* libpng: http://www.libpng.org/pub/png/libpng.html (for .PNG screenshots)
* zlib: http://www.zlib.net/
* libdumb (>=2.0.0): https://bitbucket.org/kode54/dumb (for module music)

[libShadowDive](https://github.com/omf2097/libShadowDive) is our own library for handling OMF:2097
file formats. It is probably not in any repositories by default, and so you may need to compile
it yourself. It is also possible to pull this in as a submodule; please see USE_SUBMODULES cmake
flag for that.

#### Debian
On debian, it is possible to pull some libraries using apt-get.
```
apt-get install libsdl2-dev libopenal-dev libconfuse-dev libenet-dev libargtable2-dev libxmp-dev
```

If you wish to use ogg vorbis and png support, you may also run
```
apt-get install libogg-dev libvorbis-dev libpng-dev
```

#### Mac OS X (Homebrew):
```
brew install sdl2
brew install confuse
brew install enet
brew install gettext
brew install libxmp
```

### Acquiring the sources

Latest OpenOMF source can be acquired with the "git clone" command below.

```
git clone https://github.com/omf2097/openomf.git
```

Note that the latest sources do not necessarily compile or they may have bugs. To retrieve 
the latest source that compiles but will be out of date, see the 
[releases](https://github.com/omf2097/openomf/releases) section of OpenOMF project in github.

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

| Flag                      | Description                             | Choices         | Default |
| ------------------------- | --------------------------------------- | --------------- | ------- |
| CMAKE_BUILD_TYPE          | Chooses between build types             | -/Debug/Release | -       |
| CMAKE_INSTALL_PREFIX      | Installation path                       | -               | -       |
| USE_TESTS                 | Compile unittests                       | On/Off          | Off     |
| USE_OGGVORBIS             | Selects Vorbis support                  | On/Off          | Off     |
| USE_PNG                   | Selects PNG screenshot support          | On/Off          | On      |
| USE_OPENAL                | Selects OpenAL support                  | On/Off          | On      |
| USE_DUMB                  | Selects libdumb support                 | On/Off          | Off     |
| USE_XMP                   | Selects libxmp support                  | On/Off          | On      |
| USE_SUBMODULES            | Pull in libsd as submodule              | On/Off          | On      |

Ogg Vorbis support is required if you wish to replace original OMF soundtracks with OGG files.
Otherwise the switch is optional.

For music playback, select at least one (or more) of the module player libraries. 
Available: libdumb, libxmp. Libxmp is recommended.

It is technically possible to select more than one audio sink, or none. Currently only one
audio sink is supported (OpenAL). If all audio sinks are off, then no audio will be played.
This will also of course reduce cpu usage a bit.

## 3. Data Files

OpenOMF loads the original data files from the original OMF:2097 game using libShadowDive.
Since One Must Fall 2079 is freeware, the files are obtainable for free from
[www.omf2097.com](http://www.omf2097.com/pub/files/omf/omf2097.rar).

On linux in debug mode, the OMF resource files should be put in resources/ subdirectory.

On linux in release mode, the resources should be located in ../share/games/openomf/,
relative to the openomf binary. So if your binary is in /usr/local/bin, the resources should
be put in /usr/local/share/games/openomf/. With release or testing packages,
the resources should be extracted to /usr/share/games/openomf/.

On MacOS and windows, the resources should be put into the resources/ subdirectory.

Note! Only BK,AF,PIC,TRN,PSM and DAT files are required, others may be removed to save space.

You can override these default paths by setting the OPENOMF_RESOURCE_DIR environment
variable to an absolute directory. Plugins have a similiar environment variable called
OPENOMF_PLUGIN_DIR.

## 4. Play the game!

Start the game by running the main binary. If there are problems, please contact us by either
making an issue ticket, or at #omf @ freenode in IRC (mode details in the
[README.md](https://github.com/omf2097/openomf/blob/master/README.md)).
