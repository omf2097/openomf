libShadowDive
=============

[![Build Status](https://travis-ci.org/omf2097/libShadowDive.png?branch=master)](https://travis-ci.org/omf2097/libShadowDive)

Overview
--------
libShadowDive is a library for reading and writing One Must Fall 2097 datafiles. Note! Because we don't yet know everything about OMF2097 file formats and therefore the library keeps changing all the time, libShadowDive is meant to be statically linked to OpenOMF and Omf2097-tools projects by pulling a certain version of it as a git submodule. Dynamic linking could be made to work, but there is not much point to it currently.

Features:
* HAR Data files (*.AF)
* Arena/background data files (*.BK)
* Language files (ENGLISH.DAT,GERMAN.DAT)
* Sound data file (SOUNDS.DAT)
* Characters for both big and small fonts (GRAPHCHR.DAT, CHARSMAL.DAT)
* Score file (SCORES.DAT)
* Pilot image files (*.PIC)
* Tournament data files (*.TRN)
* Character save files (*.CHR)
* Match record files (*.REC)
* Alternate palette file (ALTPALS.DAT)

Other files:
* OMF music files are in PSM module format, and can be opened with libdumb.

Dependencies
---------------

CMake is required for building this package. LibPNG is required for importing and exporting
image data. Compiling should be possible with any C99 capable C compiler (GCC and CLANG tested).

Some common package manager commands for receiving the dependencies:

Debian:
```
apt-get install cmake libpng-dev
```

Compiling
---------

To compile:

```
$ mkdir -p build
$ cd build
$ cmake -DCMAKE_RELEASE_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..
$ make
```

Documentation
-------------

Documentation can be found at [https://katajakasa.fi/projects/openomf/sd_doc/](https://katajakasa.fi/projects/openomf/sd_doc/).

You can also generate the documentation yourself by running doc/Doxyfile through doxygen (```doxygen Doxyfile```).

License
-------
MIT. Please see LICENSE file for details.

Contact
-------
Join us on [#omf](http://webchat.freenode.net?channels=omf) on [irc.freenode.net](irc://chat.freenode.net/omf) if you want to get in contact with us.
