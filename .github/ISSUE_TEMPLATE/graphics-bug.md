---
name: Graphics bug
about: When there are graphics glitches or other video problems
title: "[GRAPHICS]"
labels: ''
assignees: ''

---

Fill in as many of these fields as possible! The more data we have, the easier it will be for us to locate
and fix the issue.

## Steps to reproduce

### What were you doing?

Fill in here.

### What happened?

Fill in here.

### What did you expect to happen?

Fill in here.

## Documentation

### Platform (please complete the following information):

- OS: [e.g. Windows, MacOS, Linux]
- Version [e.g. 0.7.3, check the window title or the logfile if unsure]
- Graphics device name [e.g. AMD, nVidia or Intel graphics card]
- Graphics driver version

### Graphics environment logs

If on linux, attach elginfo (if on wayland) or glxinfo (if on Xorg) output file.

``` glxinfo -B > glxinfo.txt ```
OR 
``` eglinfo -B > eglinfo.txt ```

### Apitrace

Attach a recording using [apitrace](https://github.com/apitrace/apitrace):
1. Install apitrace and run `apitrace trace openomf`.
2. Run the game and make sure to catch the issue. Make the recording as short as possible.
3. Compress and attach the openomf.trace produced by apitrace.

Note!
- If you are using SDL2 with wayland enabled, you may need to use `apitrace trace --api=egl openomf`.
- To debug flatpak apps, see instructions at https://github.com/apitrace/apitrace/wiki/Flatpak

## Additional context

Add any other context about the problem here.
