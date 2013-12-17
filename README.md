libShadowDive
=============

[![Build Status](https://travis-ci.org/omf2097/libShadowDive.png?branch=master)](https://travis-ci.org/omf2097/libShadowDive)

Overview
--------
libShadowDive is a library for reading and writing One Must Fall 2097 datafiles. 

Features:
* HAR Data files (*.AF)
* Arena/background data files (*.BK)
* Language files (ENGLISH.DAT,GERMAN.DAT) (Only reading supported)
* Sound data file (SOUNDS.DAT) (Only reading supported)
* Characters for both big and small fonts (GRAPHCHR.DAT, CHARSMAL.DAT)
* Score file (SCORES.DAT)
* Pilot image files (*.PIC) (Only reading supported)
* Tournament data files (*.TRN) (Only reading supported)
* Character save files (*.CHR) (Only reading supported)

Following formats are still TODO:
* Character save files (*.CHR) writing
* Tournament data files (*.TRN) writing
* Pilot image files (*.PIC) writing
* Language file (ENGLISH.DAT, GERMAN.DAT) writing
* Sound file (SOUNDS.DAT) writing

Other files:
* OMF music files are in PSM module format, and can be opened with libdumb.

Compiling
---------

To compile:

```
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
