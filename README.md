OpenOMF Readme [![Build Status](https://travis-ci.org/omf2097/openomf.png?branch=master)](https://travis-ci.org/omf2097/openomf)
=======

OpenOMF is a Open Source remake of "One Must Fall 2097" by Diversions Entertainment. Since the original DOS game from 1994 still uses IPX networking and is a pain to set up, the community needed a better solution to keep playing the game we love. Together with networking, we try to make it easier to play One Must Fall in original glory on multiple platforms (Linux, Mac OSX, Windows, BSD to name a few), although it might take some time to set it up.

For mode detailed information about the project, please see [http://omf2097.github.io/openomf/](http://omf2097.github.io/openomf/).

For installing & compiling, please see [INSTALL.md](https://github.com/omf2097/openomf/blob/master/INSTALL.md).

Links
-----
* There are some videos of openomf at http://www.youtube.com/user/openomf
* We have a forum thread at http://omf.justinoakley.com/viewtopic.php?t=312299
* Our official wiki & documentation is at http://www.omf2097.com/wiki/doku.php?id=openomf:start
* We can also be reached on IRC (freenode) at #omf.

Project goals
-------------
1. At first, as faithful a game binary to the original as possible. When this is reached, we can work on new features.
2. Better game controller support (new gamepads etc.)
3. Better networking support (dosbox IPX/SPX routing is annoying to set up + the original game will run slowly). At first only 1 vs. 1 matches; later maybe full multiplayer tournaments and stuff :)
4. Support for new graphics and audio. The game already supports loading alternate audio files. Support for alternate sprites is coming after the game is otherwise working okay.
5. Enhanced tournament mode!
6. Enjoying challenges in reverse-engineering and coding :)

Dependencies
------------

* libShadowDive: https://github.com/omf2097/libShadowDive
* libdumb: https://bitbucket.org/kode54/dumb
* SDL2 (>=2.0.2): http://www.libsdl.org/download-2.0.php
* OpenAL: http://kcat.strangesoft.net/openal.html
* confuse: http://www.nongnu.org/confuse/
* Gettext (if you have problems with libintl)
* Enet: http://enet.bespin.org/

Optional:
* libvorbis: https://www.xiph.org/downloads/
* libogg: https://www.xiph.org/downloads/
* libpng: http://www.libpng.org/pub/png/libpng.html
* zlib: http://www.zlib.net/

Several of these are probably available as packages for your operating system, but some you will have to install from source.

License
-------
OpenOMF is developed under the MIT License. Please read [LICENSE](https://github.com/omf2097/openomf/blob/master/LICENSE) for more information.

Contribute
----------
You may submit patches to this project by forking this repository and making a pull request.

If you want to become a regular contributor, please contact us on [#omf](http://webchat.freenode.net?channels=omf) on [irc.freenode.net](irc://chat.freenode.net/omf).

We appreciate all kinds of contributions, whether it's big or small.

Contact
-------
Join us on [#omf](http://webchat.freenode.net?channels=omf) on [irc.freenode.net](irc://chat.freenode.net/omf) if you want to get in contact with us.
