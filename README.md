libShadowDive
=============

[![Build Status](https://travis-ci.org/omf2097/libShadowDive.png?branch=master)](https://travis-ci.org/omf2097/libShadowDive)

Overview
--------
libShadowDive is a library for reading and writing One Must Fall 2097 datafiles. 

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

CMake is required for building this package.

Some common package manager commands for receiving the dependencies:

Debian:
```
apt-get install cmake
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

License
-------
MIT. Please see LICENSE file for details.

Contact
-------
Join us on [#omf](http://webchat.freenode.net?channels=omf) on [irc.freenode.net](irc://chat.freenode.net/omf) if you want to get in contact with us.
