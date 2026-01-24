# Path handling in OpenOMF

## Terms:

- Resource path is for original game resources and additional OpenOMF resources (read-only)
- State path is for saved games, scores, logfiles, recs and screenshots (read-write)
- Config path is for the config file (read-write)

## How paths are resolved:

OpenOMF uses these, in priority order:

1. First, try OPENOMF variables:
    - OPENOMF_STATE_DIR for state path
    - OPENOMF_CONFIG_DIR for config path
    - OPENOMF_RESOURCE_DIR for resources path
2. Fallback to XDG variables [1]:
    - XDG_STATE_HOME for state path
    - XDG_CONFIG_HOME for config path
    - XDG_DATA_HOME for resource path
3. Fallback to SDL_GetPrefPath() for state and config [2].
    - On Windows, this is usually `%AppData%/OpenOMF/`
    - On linux, this should be `~/.local/share/OpenOMF`
    - On MacOSX, this should be `/Users/bob/Library/Application Support/OpenOMF/`
4. If at this point we don't have state and config dir, crash! For resource dirs, default to:
    - Linux: /usr/local/share/openomf
    - MacOS: /usr/local/share/openomf
    - Windows: Use binary directory

## Notes:

- Paths are loaded and variables resolved at game start -- no repeated getenv calls.

## Sources

[1] https://wiki.archlinux.org/title/XDG_Base_Directory
[2] https://wiki.libsdl.org/SDL2/SDL_GetPrefPath