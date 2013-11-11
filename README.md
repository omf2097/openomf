OpenOMF
=======

OpenOMF is a Open Source remake of "One Must Fall 2097" by Diversions Entertainment. Since the original DOS game from 1994 still uses IPX networking and is a pain to set up, the community needed a better solution to keep playing the game we love. Together with networking, we try to make it easier to play One Must Fall in original glory on multiple platforms (Linux, Mac OSX, Windows, BSD to name a few), although it might take some time to set it up.

Build status
------------
[![Build Status](https://travis-ci.org/omf2097/openomf.png?branch=master)](https://travis-ci.org/omf2097/openomf)

Links
-----
* There are some videos of openomf at http://www.youtube.com/user/Katajakasa
* We have a forum thread at http://omf.justinoakley.com/viewtopic.php?t=312299
* Our official wiki & documentation is at http://www.omf2097.com/wiki/doku.php?id=openomf:start
* We can also be reached on IRC (freenode) at #omf.

Dependencies
------------

* libShadowDive: https://github.com/omf2097/libShadowDive
* libdumb: https://bitbucket.org/kode54/dumb
* SDL2: http://www.libsdl.org/tmp/download-2.0.php
* OpenAL: http://kcat.strangesoft.net/openal.html
* confuse: http://www.nongnu.org/confuse/
* GLEW: http://glew.sourceforge.net/
* Gettext (if you have problems with libintl)
* Enet: http://enet.bespin.org/

Several of these are probably available as packages for your operating system, but some you will have to install from source.

Compiling
---------

To compile:

```
$ cd build
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```

Some useful CMake flags:

| Flag                      | Description                          | Choices       | Default |
| ------------------------- | ------------------------------------ | ------------- | ------- |
| CMAKE_BUILD_TYPE          | Chooses between build types          | Debug/Release | Debug   |
| USE_OGGVORBIS             | Selects Vorbis support               | On/Off        | Off     |
| USE_LTO                   | Selects Link time optimizations flag | On/Off        | Off     |

Data Files
----------
OpenOMF uses the original data files from original game using libShadowDive. Since One Must Fall 2079 is freeware the files are obtainable from http://omf2097.com/

The OMF resource files should be put in build/resources.

License
-------
OpenOMF is developed under the MIT License. Please read LICENSE for more information.

Contact
-------
Join us on [#omf](http://webchat.freenode.net?channels=omf) on [irc.freenode.net](irc://chat.freenode.net/omf) if you want to get in contact with us.
