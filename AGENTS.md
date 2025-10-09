# AI Agent Guide for OpenOMF

This document provides guidance for AI coding assistants (Claude, GPT-4, etc.) working on the OpenOMF codebase.

## Project Overview

OpenOMF is an open-source remake of "One Must Fall 2097" by Diversions Entertainment. This is a **C99 project** that uses **CMake** as its build system. The project recreates a DOS fighting game from 1994 with modern networking capabilities and cross-platform support.

**Game Description**: One Must Fall is a fighting game where two giant robots (called **HARs** - Human Assisted Robots) battle each other in various arenas. Each HAR has its own stats (arm power, leg power, armor, endurance, etc.) and is controlled by a pilot who also has individual stats and portraits. Players can compete in tournaments and save/replay matches.

- **Language**: C99 (strictly C, not C++)
- **Build System**: CMake 3.16+
- **Platforms**: Linux, macOS, Windows, BSD
- **Code Style**: Enforced via clang-format
- **Linting**: clang-tidy available

## Project Structure

```
openomf/
├── src/                    # Main source code
│   ├── audio/             # Audio backends and sources
│   ├── console/           # In-game console
│   ├── controller/        # Input handling and AI
│   ├── formats/           # Game data format parsers (AF, BK, etc.)
│   ├── game/              # Game logic, GUI, scenes
│   ├── resources/         # Resource management
│   ├── utils/             # Utility functions
│   ├── video/             # Video rendering backends
│   └── vendored/          # Third-party code (argtable3, zip, miniz)
├── tools/                 # CLI tools for game data manipulation
├── testing/               # Unit tests (CUnit)
├── shaders/               # OpenGL shaders
├── resources/             # Game resources and data files
├── cmake-scripts/         # CMake helper scripts
└── vcpkg-ports/          # Custom vcpkg ports
```

## Building the Project

**Important**: Always build in a subdirectory (like `build/`) to avoid littering the project root with build artifacts. The project `.gitignore` is configured for this workflow.

### Quick Build
```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Development Build (recommended for agents)
```bash
# Use a separate build directory for AI experiments
mkdir -p ai-build
cd ai-build
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DUSE_TESTS=ON \
      -DUSE_TOOLS=ON \
      -DUSE_FORMAT=ON \
      -DUSE_TIDY=ON \
      ..
make
```

### Running the Game
```bash
cd build  # or ai-build if you used that directory
make
./openomf
```

### Code Formatting
```bash
cd build
make clangformat
```

## Important CMake Options

| Flag                 | Description                             | Default |
|----------------------|-----------------------------------------|---------|
| CMAKE_BUILD_TYPE     | Debug/Release/RelWithDebInfo           | -       |
| USE_TESTS            | Build unit tests (requires CUnit)       | OFF     |
| USE_TOOLS            | Build data manipulation tools           | OFF     |
| USE_SANITIZERS       | Enable ASan/UBSan                       | OFF     |
| USE_FORMAT           | Enable clang-format                     | OFF     |
| USE_TIDY             | Enable clang-tidy                       | OFF     |
| USE_LIBPNG           | Enable PNG screenshot support           | ON      |
| USE_OPUSFILE         | Enable Ogg/Opus music support           | ON      |

## Code Style Guidelines

### General Rules
- **C99 standard** - Use C99 features, not C11/C17 or C++
- **4-space indentation** (enforced by clang-format)
- **Snake_case** for functions and variables
- **UPPER_CASE** for macros and constants
- **typedef structs** are common in the codebase
- **NULL checks** are critical - this is C, not Rust

### Memory Management
- **Always use `omf_malloc()`, `omf_calloc()`, `omf_realloc()`, and `omf_free()`** - Never use standard `malloc()`/`free()`
- These custom allocators have built-in abort on allocation failure - no need to check for NULL returns
- `omf_free()` automatically sets the pointer to NULL after freeing
- Use `omf_calloc()` or `memset()` to zero-initialize structures when appropriate
- Be mindful of ownership - who allocates, who frees?
- Never mix standard allocators with `omf_*` allocators

### Error Handling
- Return error codes (typically 0 for success, non-zero for error)
- Use logging macros: `log_debug()`, `log_info()`, `log_warn()`, `log_error()` (see `src/utils/log.h`)
- Critical errors should be logged before returning

### Common Patterns

**Initialization/Free Pattern**:
```c
#include "utils/allocator.h"

int object_create(object *obj, ...) {
    memset(obj, 0, sizeof(object));
    // Initialize fields with omf_malloc if needed
    obj->data = omf_malloc(size);  // No NULL check needed, aborts on failure
    return 0;
}

void object_free(object *obj) {
    // Free allocated resources with omf_free
    omf_free(obj->data);  // Automatically sets obj->data to NULL
    // No need to free obj itself if stack-allocated
}
```

**Error Handling**:
```c
int function_that_can_fail(void) {
    if(some_condition) {
        log_error("Descriptive error message: %s", reason);
        return 1;
    }
    return 0;
}
```

## Key Subsystems

### 1. Formats (`src/formats/`)
Binary file format parsers for original OMF:2097 game data. These are proprietary formats from the original DOS game:

- **AF files** (Animation Files): Contain HAR robot data, including:
  - Animation sequences for each HAR
  - Frames within each animation
  - HAR stats (arm power, leg power, armor, endurance, etc.)
  - Move data and hitboxes

- **BK files** (Background Files): Contain scene/arena data, including:
  - Arena backgrounds and stages
  - Other game scenes (menus, cutscenes, etc.)
  - Animations and frames within scenes

- **CHR files**: Tournament player save files containing:
  - Pilot data (stats, portrait)
  - HAR selection and customization
  - Tournament progress

- **TRN files**: Tournament description files containing:
  - Tournament structure and rules
  - Opponent information
  - Prize data

- **REC files**: Match recording/replay files containing:
  - Frame-by-frame input recording
  - Can be played back for match "videos"
  - Used for regression testing

- **Language files** (ENGLISH.DAT, GERMAN.DAT): Proprietary binary format containing game text translations

- **Font files** (CHARSMAL.DAT, GRAPHCHR.DAT): Bitmap fonts used in the game

- **Music**: PSM format files handled by libxmp

- **Audio**: SOUNDS.DAT contains raw PCM audio data, handled by SDL_mixer

**Important**: AF and BK files both contain animations, which contain frames. This hierarchical structure (File → Animation → Frame) is fundamental to the game's asset system.

When working with formats, check the wiki: https://github.com/omf2097/openomf/wiki

### 2. Resource Loaders (`src/resources/`)
High-level API for loading game resources. These wrap the low-level format parsers:

- **af_loader**: Load HAR animation files by ID
  ```c
  af har_file;
  load_af_file(&har_file, har_id);  // Load HAR by ID (0-9)
  ```

- **bk_loader**: Load scene/arena background files
  ```c
  bk background;
  path bk_path;
  path_from_c(&bk_path, "ARENA0.BK");
  load_bk_file(&background, &bk_path);
  ```

- **fonts**: Font loading and management (CHARSMAL.DAT, GRAPHCHR.DAT)
  ```c
  fonts_init();
  const font *f = fonts_get_font(FONT_BIG);
  const surface *glyph = font_get_surface(f, 'A');
  fonts_close();
  ```

- **languages**: Language/translation system (ENGLISH.DAT, GERMAN.DAT)
  ```c
  lang_init();
  const char *text = lang_get(LANG_STR_HAR + har_id);  // Get HAR name
  lang_close();
  ```

- **sounds_loader**: Sound effects loading (SOUNDS.DAT - raw PCM)
  ```c
  sounds_loader_init();
  char *buffer;
  int len, freq;
  sounds_loader_get(sound_id, &buffer, &len, &freq);
  sounds_loader_close();
  ```

- **pilots**: Pilot data and stats
  ```c
  pilot p;
  pilot_get_info(&p, pilot_id);
  // p.power, p.agility, p.endurance, p.color_1/2/3, p.sex
  ```

- **trnmanager**: Tournament file loading (.TRN files)
  ```c
  sd_tournament_file trn;
  trn_load(&trn, "TOURNEY1.TRN");
  ```

- **sgmanager**: Savegame management (.CHR files - pilot save data)
  ```c
  sd_chr_file chr;
  sg_load_pilot(&chr, "PILOT_NAME");  // Load by pilot name
  sg_save(&chr);                       // Save changes
  sg_delete("PILOT_NAME");             // Delete savegame
  ```

- **modmanager**: Mod support system for custom content

### 3. Game Objects (`src/game/`)
- **Scenes** (`src/game/scenes/`): Main game states
  - Arena: Where HAR battles take place
  - Melee: Quick match setup
  - Mechlab: HAR/pilot selection and customization
  - Tournament scenes

- **GUI** (`src/game/gui/`): Menu system and widgets

- **Game Objects** (`src/game/objects/`):
  - **HARs**: Giant robots with individual stats and animations
  - **Pilots**: HAR operators with portraits and stats
  - Projectiles: Weapons fired by HARs
  - Hazards: Environmental dangers in arenas
  - Scrap: Debris created during combat

### 4. Controllers (`src/controller/`)
- **Player input handling**: Keyboard, joystick, gamepad support
- **AI controller logic**: Computer-controlled HAR opponents
- **Recording/playback**: REC file recording and playback for match replays

### 5. Audio/Video Backends
Plugin-based architecture:
- **Audio backends** (`src/audio/backends/`):
  - SDL backend for audio playback
  - Music: PSM files via libxmp
  - Sound effects: Raw PCM from SOUNDS.DAT via SDL_mixer

- **Video renderers** (`src/video/renderers/`):
  - OpenGL3 renderer for graphics
  - Null renderer for headless testing (debug builds only)

## Dependencies

Core dependencies (managed via vcpkg or system package manager):
- **SDL2** (≥2.0.16): Windowing, input, audio
- **SDL2_mixer** (≥2.0.4): Audio mixing
- **libconfuse**: Configuration file parsing
- **ENet**: Networking
- **libepoxy**: OpenGL function loading
- **libxmp**: Module music playback
- **libpng**: PNG support (screenshots)
- **opusfile**: Ogg/Opus music
- **miniupnpc/natpmp**: NAT traversal (optional)

Development dependencies:
- **CUnit**: Unit testing framework

## Common Tasks for Agents

### Adding a New Feature
1. Check if tests exist - if USE_TESTS=ON, write tests
2. Follow existing patterns in similar code
3. Update documentation if adding public APIs
4. Run clang-format before committing
5. Verify build with both Debug and Release configurations

### Fixing a Bug
1. Reproduce the issue if possible
2. Add a test case if applicable
3. Check for similar issues in the codebase
4. Consider memory safety (use valgrind/asan if available)
5. Verify the fix doesn't break existing tests

### Refactoring
1. Ensure tests pass before and after
2. Maintain backward compatibility for save files and network protocol
3. Keep commits focused and atomic
4. Run clang-tidy to catch issues

### Working with Game Data
- Original OMF:2097 data files are required
- Tools in `tools/` can inspect/modify game data
- Use `USE_TOOLS=ON` to build data manipulation tools
- See PATHS.md for resource file locations

## Testing

### Unit Tests (CUnit)
Located in `testing/` directory. Written using the **CUnit** testing framework. Build with `-DUSE_TESTS=ON`.

```bash
# IMPORTANT: Always test with sanitizers to catch memory leaks and UB
rm -r ai-build-sanitizers
mkdir ai-build-sanitizers
cd ai-build-sanitizers
cmake -DUSE_TESTS=ON -DUSE_SANITIZERS=ON ..
make
./openomf_test_main
```

**Note**: Always run tests with sanitizers enabled (`-DUSE_SANITIZERS=ON`) to detect memory leaks, use-after-free, and undefined behavior. This is critical for C code quality.

**Writing Tests**:
```c
#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

// Test function
void test_something(void) {
    CU_ASSERT_EQUAL(2 + 2, 4);
    CU_ASSERT_TRUE(some_condition);
    CU_ASSERT_PTR_NOT_NULL(ptr);
}

// In test suite setup
CU_pSuite suite = CU_add_suite("MyTestSuite", NULL, NULL);
CU_add_test(suite, "test something", test_something);
```

Common CUnit assertions:
- `CU_ASSERT(expression)` - Assert expression is true
- `CU_ASSERT_EQUAL(actual, expected)` - Assert equality
- `CU_ASSERT_NOT_EQUAL(actual, expected)` - Assert inequality
- `CU_ASSERT_PTR_NULL(ptr)` - Assert pointer is NULL
- `CU_ASSERT_PTR_NOT_NULL(ptr)` - Assert pointer is not NULL
- `CU_ASSERT_STRING_EQUAL(actual, expected)` - Assert string equality

### Integration Tests
Python-based tests in `pytest/`:
```bash
./run_pytest.sh
```

### REC File Regression Tests
Located in `rectests/` directory. These tests use the REC file format to ensure game mechanics work correctly:

```bash
./run_rectests.sh
```

**How REC tests work**:
- REC files record frame-by-frame inputs during matches
- **Assertions can be embedded** in the REC move stream to verify game state
- When a REC file is played back, assertions check:
  - HAR position, health, and state
  - Animation frames
  - Hit detection and collision results
  - Expected outcomes of specific move sequences
- This ensures moves produce deterministic, expected results
- Excellent for regression testing game mechanics and physics

**Writing REC tests**:
- Record a match with specific moves you want to test
- Insert assertions into the REC file move stream at key points
- The test passes if all assertions succeed during playback
- Failures indicate game mechanics have changed unexpectedly

## Debugging

### Debug Mode Features
- NULL audio and video backends for headless testing
- Extensive logging (see `src/utils/log.h`)
- Debug console (see DEBUG-KEYS.md)
- Frame-perfect recording/playback (REC files)

### Useful Flags
```bash
# Enable sanitizers
cmake -DUSE_SANITIZERS=ON -DUSE_FATAL_SANITIZERS=ON ..

# Run with LeakSanitizer suppression
LSAN_OPTIONS=suppressions=../lsan.supp ./openomf
```

### Debug Keys
See DEBUG-KEYS.md for in-game debug shortcuts.

## Contributing Guidelines

Before submitting changes:
1. **Format code**: `make clangformat` (with USE_FORMAT=ON)
2. **Run linter**: Use USE_TIDY=ON during development
3. **Test**: Ensure all tests pass
4. **No build errors**: Both Debug and Release builds must succeed
5. **Follow PR workflow**: See CONTRIBUTING.md

### Commit Messages
- Be descriptive but concise
- Reference issue numbers if applicable
- Use imperative mood ("Add feature" not "Added feature")

### Code Review
- PRs must pass CI checks
- Must be formatted with clang-format
- Must be verified with clang-tidy
- Small changes can be self-merged by maintainers
- Large changes need review and approval

## Important Caveats

### Vendored Code
- **NEVER modify code in `src/vendored/`** - This is third-party code (argtable3, zip, miniz)
- These libraries have their own licenses and update cycles
- If changes are needed, consider upstreaming to the original project

### Platform-Specific Code
- Use `#ifdef WIN32` for Windows-specific code
- Use `#ifdef __MINGW32__` for MinGW-specific code
- Test cross-platform builds when possible

### Networking
- Game uses ENet for networking (UDP-based)
- Network protocol compatibility is critical
- See NETWORKING.md for details

### File Formats
- Backward compatibility with original OMF:2097 data is essential
- Don't break existing save files (pilot data, configuration)
- Test with actual game data files

### Performance
- This is a 1994 game remake - performance is usually not critical
- But avoid unnecessary allocations in hot paths (rendering, game loop)
- Profile before optimizing

## Resources

- **Project Website**: http://www.openomf.org/
- **GitHub**: https://github.com/omf2097/openomf
- **Wiki**: https://github.com/omf2097/openomf/wiki
- **Discord**: https://discord.gg/7CPPzab
- **Discussion**: https://github.com/omf2097/openomf/discussions/413

## Quick Reference

### Memory Allocation
```c
#include "utils/allocator.h"

// Use these instead of malloc/calloc/realloc/free
int *ptr = omf_malloc(sizeof(int) * 10);
int *zeroed = omf_calloc(10, sizeof(int));
ptr = omf_realloc(ptr, sizeof(int) * 20);
omf_free(ptr);  // Automatically sets ptr to NULL
```

### Logging
```c
#include "utils/log.h"

log_debug("Debug message: %d", value);
log_info("Info message");
log_warn("Warning: something might be wrong");
log_error("Error occurred: %s", error_msg);
```

### String Operations (`str`)
```c
#include "utils/str.h"

// Create and manipulate strings
str my_str;
str_create(&my_str);
str_from_c(&my_str, "Hello");
str_append_c(&my_str, " World");
str_toupper(&my_str);
const char *c_str = str_c(&my_str);  // Get C string
str_free(&my_str);

// Format strings
str formatted;
str_from_format(&formatted, "Value: %d", 42);
str_free(&formatted);
```

### Dynamic Arrays (`vector`)
```c
#include "utils/vector.h"

vector my_vec;
vector_create(&my_vec, sizeof(int));
int value = 42;
vector_append(&my_vec, &value);
int *item = vector_get(&my_vec, 0);
vector_free(&my_vec);
```

### Linked Lists (`list`)
```c
#include "utils/list.h"

list my_list;
list_create(&my_list);
int value = 42;
list_append(&my_list, &value, sizeof(int));
list_prepend(&my_list, &value, sizeof(int));
list_free(&my_list);
```

### Hash Maps (`hashmap`)
```c
#include "utils/hashmap.h"

hashmap map;
hashmap_create(&map);

// String keys
int value = 42;
hashmap_put_str(&map, "key", &value, sizeof(int));
void *result;
unsigned int len;
hashmap_get_str(&map, "key", &result, &len);

// Integer keys
hashmap_put_int(&map, 123, &value, sizeof(int));
hashmap_get_int(&map, 123, &result, &len);

hashmap_free(&map);
```

### Ring Buffer (`ringbuffer`)
```c
#include "utils/ringbuffer.h"

ring_buffer rb;
rb_create(&rb, 1024);
char data[100] = "test";
rb_write(&rb, data, 4);
char read_buf[100];
rb_read(&rb, read_buf, 4);
rb_free(&rb);
```

### Path Operations (`path`)
```c
#include "utils/path.h"

// Build and manipulate file paths
path p;
path_from_parts(&p, "resources", "sprites", "file.DAT");
const char *path_str = path_c(&p);

// Check path properties
if(path_exists(&p) && path_is_file(&p)) {
    FILE *fp = path_fopen(&p, "rb");
    // ... use file
    fclose(fp);
}

// Get path components
str filename, extension;
str_create(&filename);
str_create(&extension);
path_filename(&p, &filename);
path_ext(&p, &extension);
str_free(&filename);
str_free(&extension);
```

### 2D Vectors (`vec`)
```c
#include "utils/vec.h"

// Integer vectors
vec2i pos = vec2i_create(10, 20);
vec2i offset = vec2i_create(5, 5);
vec2i new_pos = vec2i_add(pos, offset);

// Float vectors
vec2f fpos = vec2f_create(10.5f, 20.5f);
float distance = vec2f_dist(fpos, vec2f_create(0.0f, 0.0f));
vec2f normalized = vec2f_norm(fpos);
```

### Iterators
```c
#include "utils/iterator.h"
#include "utils/list.h"
#include "utils/vector.h"

// Iterate over a vector
vector my_vec;
vector_create(&my_vec, sizeof(int));
// ... populate vector ...

iterator it;
vector_iter_begin(&my_vec, &it);
int *item;
foreach(it, item) {
    // Use item
}

// Iterate over a list
list my_list;
list_create(&my_list);
// ... populate list ...

list_iter_begin(&my_list, &it);
foreach(it, item) {
    // Use item
}

// Reverse iteration
list_iter_end(&my_list, &it);
foreach_reverse(it, item) {
    // Use item in reverse
}
```

### Crash Handling
```c
#include "utils/crash.h"

// For unrecoverable errors (terminates program with diagnostic info)
if(critical_failure) {
    crash("Critical failure occurred");
}

// With additional context
crash_with_args("Failed to load resource: %s", filename);
```

## License

OpenOMF is licensed under the MIT License. Ensure any contributions are compatible with this license.

## Final Notes for Agents

- **Read before writing**: Understand existing patterns before adding code
- **Test thoroughly**: This is a game - bugs are visible and frustrating
- **Respect the C99 standard**: Don't use C++ features or newer C standards
- **Memory safety first**: Leaks and crashes are unacceptable
- **When in doubt, ask**: Check existing code or documentation

Happy coding!
