OpenOMF
=======

OpenOMF is a Open Source remake of "One Must Fall 2097" by Diversions Entertainment.
Since the original DOS game from 1994 still uses IPX networking and is a pain to set up, the community needed a better solution to keep playing the game we love.
Together with networking, we try to make it easier to play One Must Fall in original glory on multiple platforms (Linux, Mac OSX, Windows, BSD to name a few), although it might take some time to set it up.

Dependencies
------------

* libShadowDive: https://github.com/omf2097/libShadowDive
* libdumb: https://github.com/omf2097/libdumb-kataja
* SDL2: http://www.libsdl.org/tmp/SDL-2.0.tar.gz
* OpenAL: http://kcat.strangesoft.net/openal.html
* confuse: http://www.nongnu.org/confuse/
* GLEW: http://glew.sourceforge.net/
* Gettext (if you have problems with libintl)
* Chipmunk: http://chipmunk-physics.net/

Several of these are probably available as packages for your operating system, but some you will
have to install from source.

Compiling
---------

To compile:

```
$ cd build
$ cmake ..
$ make
```

Data Files
----------
OpenOMF uses the original data files from original game using libShadowDive.
Since One Must Fall 2079 is freeware the files are obtainable from http://omf2097.com/

The OMF resource files should be put in build/resources.

License
-------
OpenOMF is developed under the MIT License. Please read LICENSE for more information.

Contact
-------
Join us on [#omf](http://webchat.freenode.net?channels=omf) on [irc.freenode.net](irc://chat.freenode.net/omf) if you want to get in contact with us.
