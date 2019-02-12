libShadowDive
=============

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

CMake is required for building this package. LibPNG is required for importing and exporting
image data. Compiling should be possible with any C99 capable C compiler (GCC and CLANG tested).

License
-------
Copyright (C) 2013-2014 Tuomas Virtanen, Andrew Thompson

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.