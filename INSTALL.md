OpenOMF Installation
====================

Note! If you wish to compile OpenOMF from sources yourself, begin from 1). If you are however downloading a binary release package, begin from 4).

1. Dependencies
---------------

Most of the libraries that OpenOMF requires are pretty standard and often easily available in eg. linux distro repositories. However, you may need to download and install [libShadowDive](https://github.com/omf2097/libShadowDive) and the latest [libdumb](https://bitbucket.org/kode54/dumb). Your distro may have the packages for at least libdumb, but those are likely old and may not work with OpenOMF.

Some common packagemanager commands for receiving the other deps below:

Debian:
```
apt-get install libsdl2-dev libopenal-dev libconfuse-dev libenet-dev
```

Mac OS X (Homebrew):
```
brew install sdl2
brew install confuse
brew install enet
brew install gettext
```

Gentoo:
We have an experimental gentoo portage overlay at https://github.com/omf2097/openomf-overlay

2. Acquiring the sources
------------------------

Latest OpenOMF source can be acquired with the "git clone" command below. 

```
git clone https://github.com/omf2097/openomf.git
```

Note that the latest sources do not necessarily compile or they may have bugs. To retrieve the latest source that compiles but will be out of date, see the [releases](https://github.com/omf2097/openomf/releases) section of OpenOMF project in github.

3. Compiling
------------

To compile:

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
| USE_OGGVORBIS             | Selects Vorbis support                  | On/Off          | Off     |
| USE_PNG                   | Selects PNG screenshot support          | On/Off          | Off     |
| USE_OPENAL                | Selects OpenAL support                  | On/Off          | On      |
| USE_DUMB                  | Selects libdumb support                 | On/Off          | On      |
| USE_MODPLUG               | Selects libmodplug support              | On/Off          | Off     |
| USE_SUBMODULES            | Pull in libdumb and libsd as submodules | On/Off          | On      |
| USE_RELEASE_SUBMODULES    | Build libdumb and libsd in Release mode | On/Off          | Off     |

Ogg Vorbis support is required if you wish to replace original OMF soundtracks with OGG files. Otherwise the switch is optional.

Only one module playback library should be enabled. So either do USE_DUMB=On or USE_MODPLUG=On, but not both. If both are selected, only dumb will be used. If USE_SUBMODULES is enabled, USE_DUMB=On and USE_MODPLUG=Off will be forced.

It is technically possible to select more than one audio sink, or none. Currently only one audio sink is supported (OpenAL). If all audio sinks are off, then no audio will be played. This will also of course reduce cpu usage a bit.

4. Data Files
-------------
OpenOMF loads the original data files from the original OMF:2097 game using libShadowDive. Since One Must Fall 2079 is freeware, the files are obtainable for free from [www.omf2097.com](http://www.omf2097.com/pub/files/omf/omf2097.rar).

On linux in debug mode, or on windows in any mode, the OMF resource files should be put in resources/ subdirectory. Only BK,AF,PIC,TRN,PSM and DAT files are required, others may be removed.

On linux in release mode, the resources should be located in ../share/openomf/. So if your binary is
in /usr/local/bin, the resources should be put in /usr/local/share/openomf/.

On mac, the resources should be put into the resources/ subdirectory.

5. Play the game!
-----------------
Start the game by running the main binary. If there are problems, please contact us by either making an issue ticket, or at #omf @ freenode in IRC (mode details in the [README.md](https://github.com/omf2097/openomf/blob/master/README.md)).
