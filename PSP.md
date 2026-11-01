# PSP Build and Testing

Set `PSPDEV` to your PSP SDK installation and include `$PSPDEV/bin` in `PATH`.
The PSP cross-compiler and native host C compiler, SDL2 headers, CMake and `unzip`
are required. The PSP configure step downloads the official
[OpenOMF artwork archive](https://www.omf2097.com/pub/files/omf/openomf-icons.zip),
extracts `omf-logo/omf-144x80.png`, and caches it in the build directory.
Set `OMF_PSP_ICON_PATH` to override this image. CMake builds only the EBOOT
binary, consistent with other platforms. Original game assets are staged
separately for local testing or CI release packaging.

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

The native CMake build generates language catalogs with the same translation
counts as desktop builds, without cross-compiling host tools. The PSP build
creates `build/psp/EBOOT.PBP`. The separate packaging command
assembles `build/psp/PSP/GAME/OpenOMF/` with its `resources/` directory and
`build/psp/openomf-psp-sdl-null.zip`. Copy the game directory to the PSP Memory
Stick's `PSP/GAME/` directory (or PPSSPP's Memory Stick equivalent) to run it.
To use previously downloaded original assets, set
`OMF_ASSET_ARCHIVE=/path/to/openomf-assets.zip` for the packaging command.
`RelWithDebInfo` keeps debug symbols; this milestone intentionally uses a NULL
renderer and therefore draws no gameplay graphics.
