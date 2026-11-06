# PSP Development Build

The current PSP target uses SDL2 audio and a NULL video renderer. It is intended
for developers testing the port, not for playing the game.

Install the PSP SDK, a host C compiler, SDL2 development headers, CMake and
`unzip`. Set `PSPDEV` and add `$PSPDEV/bin` to `PATH`.

Before configuring, put the PSP icon from the
[official OpenOMF icons](https://www.omf2097.com/pub/files/omf/openomf-icons.zip)
at `build/psp/icon0.png`. CMake requires that file but does not download it;
CI prepares it automatically. Alternatively, pass
`-DOMF_PSP_ICON_PATH=/path/to/icon0.png` to `psp-cmake`.

```sh
cmake -S . -B build/host -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DUSE_MINIUPNPC=OFF -DUSE_NATPMP=OFF \
  -DUSE_CONFUSE=OFF -DUSE_ZIP=OFF -DENABLE_MODMANAGER=OFF \
  -DOMF_AUDIO_BACKENDS=null -DOMF_VIDEO_RENDERERS=null
cmake --build build/host --target build_languages --parallel
psp-cmake -S . -B build/psp -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DUSE_MINIUPNPC=OFF -DUSE_NATPMP=OFF -DUSE_ARGTABLE=OFF \
  -DUSE_CONFUSE=OFF -DUSE_ZIP=OFF -DENABLE_MODMANAGER=OFF \
  -DBUILD_LANGUAGES=OFF -DOMF_AUDIO_BACKENDS=sdl \
  -DOMF_VIDEO_RENDERERS=null -DOMF_STRIP_GNU_BINARIES=OFF
cmake --build build/psp --parallel
sh packaging/psp/package.sh build/psp build/host/resources
```

The host build generates the language files from the current source. The PSP
build produces `build/psp/EBOOT.PBP`; packaging adds the game data and languages
to `build/psp/openomf-psp.zip`. To test startup and audio, copy
`build/psp/PSP/GAME/OpenOMF/` to `PSP/GAME/` on a PSP or PPSSPP Memory Stick.
