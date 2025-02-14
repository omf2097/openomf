cmake_minimum_required(VERSION 3.16)

# Options
OPTION(USE_TESTS "Build unittests" OFF)
OPTION(USE_TOOLS "Build tools" OFF)
OPTION(USE_SANITIZERS "Enable Asan and Ubsan" OFF)
OPTION(USE_FATAL_SANITIZERS "Make Asan and Ubsan errors fatal" OFF)
OPTION(USE_TIDY "Use clang-tidy for checks" OFF)
OPTION(USE_FORMAT "Use clang-format for checks" OFF)
OPTION(BUILD_LANGUAGES "Build Language Files" ON)

OPTION(USE_MINIUPNPC "Use miniupnpc for port forwarding" OFF)
OPTION(USE_NATPMP "Use natpmp for port forwarding" OFF)

# vcpkg features must be set before project()
if(USE_TESTS)
    list(APPEND VCPKG_MANIFEST_FEATURES use-tests)
endif()
if(USE_MINIUPNPC)
    list(APPEND VCPKG_MANIFEST_FEATURES miniupnpc)
endif()
if(USE_NATPMP)
    list(APPEND VCPKG_MANIFEST_FEATURES natpmp)
endif()

project(OpenOMF C)

set(USE_PCH ON)
# clang-tidy only works with clang when PCH is enabled
if(USE_PCH AND USE_TIDY AND NOT CMAKE_C_COMPILER_ID MATCHES ".*Clang")
    message(WARNING "clang-tidy cannot consume precompiled headers from non-clang compiler. Disabling PCH at the cost of higher build times.")
    set(USE_PCH OFF)
endif()

function(omf_target_precompile_headers)
    if(NOT USE_PCH)
        return()
    endif()
    target_precompile_headers(${ARGN})
endfunction()

# These flags are used for all builds
set(CMAKE_C_STANDARD 11)
if(MSVC)
    # remove cmake's default /W3 to avoid warning D9025
    string(REGEX REPLACE "/W3" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /permissive- /W4 /WX")
    # Disable select warnings:
    # C4100: unreferenced formal parameter
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4100")
    # C4456: declaration hides previous local declaration
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4456")
    # C4459: declaration hides previous global declaration
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4459")
    # C4244: implicit conversion, possible loss of data
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4244")
    # C4267: implicit conversion from size_t, possible loss of data
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4267")
    # C4389: 'equality-operator' : signed/unsigned mismatch
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /wd4389")
    # _CRT_SECURE_NO_WARNINGS: disable warnings when calling functions like strncpy or sscanf
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_CRT_SECURE_NO_WARNINGS")
    # _CRT_NONSTDC_NO_WARNINGS: disable warnings when calling POSIX functions like fileno
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D_CRT_NONSTDC_NO_WARNINGS")

    # Error on certain warnings:
    # C4013: call to undefined function
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we4013")
    # C4146: unary minus operator applied to unsigned type, result still unsigned
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we4146")
    # C4245: signed const to unsigned
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we4245")
    # C4255: function declaration missing arguments
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we4255")
    # C4305: truncation from double to float
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we4305")
    # C4701: potentially uninitialized local variable used
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we4701")
    # C4668: undefined symbol used as compiler directive (will evaluate to 0).
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we4668")
    # C4702: unreachable code
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /we4702")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wformat -pedantic -Wvla")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -Werror -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g -O2 -fno-omit-frame-pointer -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Os -DNDEBUG")
endif()

# Enable AddressSanitizer if requested
if(USE_SANITIZERS)
    if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        add_compile_options("/fsanitize=address")
        message(STATUS "Development: Asan enabled. Ubsan is unsupported by MSVC.")
    else()
        add_compile_options("-fsanitize=address,undefined")
        add_link_options("-fsanitize=address,undefined")
        if(USE_FATAL_SANITIZERS)
            add_compile_options("-fno-sanitize-recover=all")
            add_link_options("-fno-sanitize-recover=all")
            message(STATUS "Development: Asan and Ubsan errors will be fatal")
        endif()
        message(STATUS "Development: Asan and Ubsan enabled")
    endif()
else()
    message(STATUS "Development: Asan and Ubsan disabled")
endif()

# match vcpkg crt linkage
if(MSVC AND VCPKG_TARGET_TRIPLET MATCHES "-windows-static$")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
elseif(MSVC AND VCPKG_TARGET_TRIPLET MATCHES "-windows-static-md$")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# See if we have Git, and use it to fetch current SHA1 hash
find_package(Git)
if(GIT_FOUND)
    message(STATUS "Git found: ${GIT_EXECUTABLE}")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} "rev-parse" "HEAD"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE SHA1_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Git SHA1 Hash: ${SHA1_HASH}")
    set_source_files_properties(src/main.c PROPERTIES COMPILE_DEFINITIONS SHA1_HASH="${SHA1_HASH}")

    # git describe uses glob to match the tag, so no repeated character classes.
    execute_process(
        COMMAND ${GIT_EXECUTABLE} "describe" "--tags" "--match" "[0-9].[0-9].[0-9]" "--match" "[0-9].[0-9].[0-9][0-9]" "--match" "[0-9].[0-9].[0-9]-*" "--match" "[0-9].[0-9].[0-9][0-9]-*"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} "describe" "--tags" "--match" "[0-9].[0-9].[0-9]" "--match" "[0-9].[0-9].[0-9][0-9]" "--match" "[0-9].[0-9].[0-9]-*" "--match" "[0-9].[0-9].[0-9][0-9]-*" "--no-abbrev"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION_NO_ABBREV
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    string(REGEX MATCH "^([0-9]+)\\.([0-9]+)\\.([0-9]+)(-.*)?" GIT_VERSION ${GIT_VERSION})
    set(VERSION_MAJOR ${CMAKE_MATCH_1})
    set(VERSION_MINOR ${CMAKE_MATCH_2})
    set(VERSION_PATCH ${CMAKE_MATCH_3})
    set(VERSION_LABEL ${CMAKE_MATCH_4})

    string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+(-.*)?" GIT_VERSION_NO_ABBREV ${GIT_VERSION_NO_ABBREV})
    set(VERSION_NO_ABBREV_LABEL ${CMAKE_MATCH_1})
    if(NOT "${VERSION_LABEL}" STREQUAL "" AND "${VERSION_NO_ABBREV_LABEL}" STREQUAL "")
        math(EXPR VERSION_PATCH "${VERSION_PATCH} + 1")
    endif()

    set(VERSION_DEFINITIONS V_MAJOR=${VERSION_MAJOR} V_MINOR=${VERSION_MINOR} V_PATCH=${VERSION_PATCH} V_LABEL="${VERSION_LABEL}")
    set_source_files_properties(src/game/utils/version.c
      PROPERTIES COMPILE_DEFINITIONS "${VERSION_DEFINITIONS}")

    set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}${VERSION_LABEL})
    message(STATUS "Version: ${VERSION}")
endif()

if(WIN32)
    # prevent Windows.h from automatically defining as many macros
    add_definitions(-DWIN32_LEAN_AND_MEAN)

    if(NOT MINGW)
        # set source charset & runtime charset to utf-8
        add_compile_options("/utf-8")
    endif()
endif()

# openomf core dependencies
include("cmake-scripts/Dependencies.cmake")

# Find OpenOMF core sources
file(GLOB_RECURSE OPENOMF_SRC
    LIST_DIRECTORIES OFF
    CONFIGURE_DEPENDS
    RELATIVE ${CMAKE_SOURCE_DIR}
    "src/*/*.c" "src/*/*.h"
)

# Remove all player plugin source code from OPENOMF_SRC
list(FILTER OPENOMF_SRC EXCLUDE REGEX "^src/audio/backends/.*/")
# Enable "NULL" player in debug builds, for automated testing
list(APPEND OPENOMF_SRC
    "$<$<CONFIG:Debug>:src/audio/backends/null/null_backend.c>"
    "$<$<CONFIG:Debug>:src/audio/backends/null/null_backend.h>"
)
list(APPEND AUDIO_C_DEFINES "$<$<CONFIG:Debug>:ENABLE_NULL_AUDIO_BACKEND>")
# and enable select render plugins
set(ENABLED_AUDIO_BACKEND_PLUGINS sdl)
foreach (PLUGIN ${ENABLED_AUDIO_BACKEND_PLUGINS})
    # add render plugin sources
    file(GLOB_RECURSE PLUGIN_SRC
        LIST_DIRECTORIES OFF
        CONFIGURE_DEPENDS
        RELATIVE ${CMAKE_SOURCE_DIR}
        "src/audio/backends/${PLUGIN}/*.c"
        "src/audio/backends/${PLUGIN}/*.h"
    )
    list(APPEND OPENOMF_SRC ${PLUGIN_SRC})
    # hook render plugin into video.c
    string(TOUPPER "${PLUGIN}" PLUGIN_UPPER)
    list(APPEND AUDIO_C_DEFINES "ENABLE_${PLUGIN_UPPER}_AUDIO_BACKEND")
    message(STATUS "Enabled audio backend plugin '${PLUGIN}'")
endforeach ()
set_source_files_properties("src/audio/audio.c" PROPERTIES COMPILE_DEFINITIONS "${AUDIO_C_DEFINES}")

# Remove all render plugin source code from OPENOMF_SRC
list(FILTER OPENOMF_SRC EXCLUDE REGEX "^src/video/renderers/.*/")
# Enable "NULL" renderer in debug builds, for automated testing
list(APPEND OPENOMF_SRC
  "$<$<CONFIG:Debug>:src/video/renderers/null/null_renderer.c>"
  "$<$<CONFIG:Debug>:src/video/renderers/null/null_renderer.h>"
)
list(APPEND VIDEO_C_DEFINES "$<$<CONFIG:Debug>:ENABLE_NULL_RENDERER>")
# and enable select render plugins
set(ENABLED_RENDER_PLUGINS opengl3)
foreach(PLUGIN ${ENABLED_RENDER_PLUGINS})
    # add render plugin sources
    file(GLOB_RECURSE PLUGIN_SRC
        LIST_DIRECTORIES OFF
        CONFIGURE_DEPENDS
        RELATIVE ${CMAKE_SOURCE_DIR}
        "src/video/renderers/${PLUGIN}/*.c"
        "src/video/renderers/${PLUGIN}/*.h"
    )
    list(APPEND OPENOMF_SRC ${PLUGIN_SRC})
    # hook render plugin into video.c
    string(TOUPPER "${PLUGIN}" PLUGIN_UPPER)
    list(APPEND VIDEO_C_DEFINES "ENABLE_${PLUGIN_UPPER}_RENDERER")
    message(STATUS "Enabled video backend plugin '${PLUGIN}'")
endforeach()
set_source_files_properties("src/video/video.c" PROPERTIES COMPILE_DEFINITIONS "${VIDEO_C_DEFINES}")

# Build core sources first as an object library
# this can then be reused in tests and main executable to speed things up
add_library(openomf_core OBJECT ${OPENOMF_SRC})
target_compile_definitions(openomf_core PUBLIC "$<$<CONFIG:Debug>:DEBUGMODE>")
omf_target_precompile_headers(openomf_core PUBLIC
    "<SDL.h>"
    "<enet/enet.h>"
    "<epoxy/gl.h>"
)
target_link_libraries(openomf_core PRIVATE
    openomf::confuse
    openomf::enet
    openomf::epoxy
    openomf::png
    openomf::SDL2
    openomf::SDL2_mixer
    openomf::xmp
)
if(USE_MINIUPNPC)
    target_link_libraries(openomf_core PUBLIC
        openomf::miniupnpc
    )
endif()
if(USE_NATPMP)
    target_link_libraries(openomf_core PUBLIC
        openomf::natpmp
    )
endif()
list(APPEND CORELIBS openomf_core)

if(WIN32 AND NOT MINGW)
    omf_target_precompile_headers(openomf_core PUBLIC
        "<windows.h>"
    )
endif()

# Set icon for windows executable
if(WIN32)
    SET(ICON_RESOURCE "resources/icons/openomf.rc")
endif()

# Only strip on GCC (clang does not appreciate)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,-s")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Wl,-s")
endif()

set(COREINCS
    src
    ${CMAKE_CURRENT_BINARY_DIR}/src/
)

# MingW build should add mingw32 lib
if(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
endif()

# On windows, add winsock2, winmm, and shlwapi
if(WIN32)
    set(CORELIBS ${CORELIBS} ws2_32 winmm shlwapi)
endif()

# On unix platforms, add libm (sometimes needed, it seems)
if(UNIX)
    SET(CORELIBS ${CORELIBS} m)
endif()

# Set include directories for all builds
include_directories(${COREINCS})

# Build the game binary
add_executable(openomf src/main.c src/engine.c ${ICON_RESOURCE})
set_property(TARGET openomf PROPERTY
    VS_DEBUGGER_ENVIRONMENT "OPENOMF_SHADER_DIR=${CMAKE_CURRENT_BINARY_DIR}/shaders
OPENOMF_RESOURCE_DIR=${CMAKE_CURRENT_BINARY_DIR}/resources")
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT openomf)

# This is used to copy shader files from source directory to the binary output directory during compile.
add_custom_target(
    copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
)
add_dependencies(openomf copy_shaders)

# Build tools if requested
set(TOOL_TARGET_NAMES)
if(BUILD_LANGUAGES OR USE_TOOLS)
    add_executable(languagetool tools/languagetool/main.c)
    list(APPEND TOOL_TARGET_NAMES languagetool)
endif()
if (USE_TOOLS)
    add_executable(bktool tools/bktool/main.c
        tools/shared/animation_misc.c
        tools/shared/conversions.c)
    add_executable(aftool tools/aftool/main.c
        tools/shared/animation_misc.c
        tools/shared/conversions.c)
    add_executable(soundtool tools/soundtool/main.c)
    add_executable(afdiff tools/afdiff/main.c)
    add_executable(rectool tools/rectool/main.c tools/shared/pilot.c)
    add_executable(pcxtool tools/pcxtool/main.c)
    add_executable(pictool tools/pictool/main.c)
    add_executable(scoretool tools/scoretool/main.c)
    add_executable(trntool tools/trntool/main.c tools/shared/pilot.c)
    add_executable(altpaltool tools/altpaltool/main.c)
    add_executable(chrtool tools/chrtool/main.c tools/shared/pilot.c)
    add_executable(fonttool tools/fonttool/main.c)
    add_executable(setuptool tools/setuptool/main.c tools/shared/pilot.c)
    add_executable(stringparser tools/stringparser/main.c)

    list(APPEND TOOL_TARGET_NAMES
        bktool
        aftool
        soundtool
        afdiff
        rectool
        pcxtool
        pictool
        scoretool
        trntool
        altpaltool
        fonttool
        chrtool
        setuptool
        stringparser
    )
    message(STATUS "Development: CLI tools enabled")
else()
    message(STATUS "Development: CLI tools disabled")
endif()

# Linting via clang-tidy
if(USE_TIDY)
    set_target_properties(openomf PROPERTIES C_CLANG_TIDY "clang-tidy")
    set_target_properties(openomf_core PROPERTIES C_CLANG_TIDY "clang-tidy")
    foreach(TARGET ${TOOL_TARGET_NAMES})
        set_target_properties(${TARGET} PROPERTIES C_CLANG_TIDY "clang-tidy")
    endforeach()
    message(STATUS "Development: clang-tidy enabled")
else()
    message(STATUS "Development: clang-tidy disabled")
endif()

# Formatting via clang-format
if(USE_FORMAT)
    include(cmake-scripts/ClangFormat.cmake)
    file(GLOB_RECURSE SRC_FILES
        LIST_DIRECTORIES OFF
        CONFIGURE_DEPENDS
        RELATIVE ${CMAKE_SOURCE_DIR}
        "src/*.h"
        "src/*.c"
        "tools/*.c"
        "tools/*.h"
        "testing/*.c"
        "testing/*.h"
    )
    clangformat_setup(${SRC_FILES})
    message(STATUS "Development: clang-format enabled")
else()
    message(STATUS "Development: clang-format disabled")
endif()

if(WIN32)
    # Don't show console in windows release builds (but enable if debug mode)
    set_target_properties(openomf PROPERTIES WIN32_EXECUTABLE "$<IF:$<CONFIG:Debug>,OFF,ON>")

    # Make sure console apps are always in terminal output mode.
    foreach(TARGET ${TOOL_TARGET_NAMES})
        set_target_properties(${TARGET} PROPERTIES WIN32_EXECUTABLE OFF)
    endforeach()
endif()

if(MINGW)
    # Use static libgcc when on mingw
    target_link_options(openomf PRIVATE -static-libgcc)
    foreach(TARGET ${TOOL_TARGET_NAMES})
        target_link_options(${TARGET} PRIVATE -static-libgcc)
        set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "-mconsole")
    endforeach()

    # Don't show console on mingw in release builds (but enable if debug mode)
    if (NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set_target_properties(openomf PROPERTIES LINK_FLAGS "-mwindows")
    else ()
        set_target_properties(openomf PROPERTIES LINK_FLAGS "-mconsole")
    endif ()
endif()

# Make sure libraries are linked
target_link_libraries(openomf PRIVATE ${CORELIBS})
target_link_libraries(openomf PRIVATE openomf::argtable openomf::SDL2main openomf::epoxy)
foreach(TARGET ${TOOL_TARGET_NAMES})
    target_link_libraries(${TARGET} PRIVATE ${CORELIBS})
    target_link_libraries(${TARGET} PRIVATE openomf::argtable openomf::SDL2main openomf::epoxy)
endforeach()

# Testing stuff
if(CUNIT_FOUND)
    enable_testing()

    file(GLOB_RECURSE TEST_SRC
        LIST_DIRECTORIES OFF
        CONFIGURE_DEPENDS
        RELATIVE ${CMAKE_SOURCE_DIR}
        "testing/*.c"
    )

    add_executable(openomf_test_main ${TEST_SRC})

    # This makes loading test resources possible
    target_compile_definitions(openomf_test_main PRIVATE
                               TESTS_ROOT_DIR="${CMAKE_SOURCE_DIR}/testing")

    target_include_directories(openomf_test_main PRIVATE testing/ src/)
    target_link_libraries(openomf_test_main ${CORELIBS} openomf::SDL2main openomf::epoxy openomf::cunit)

    if(MSVC)
        target_precompile_headers(openomf_test_main PRIVATE "<stdio.h>")
    endif()

    if(MINGW)
        # Always build as a console executable with mingw
        set_target_properties(openomf_test_main PROPERTIES LINK_FLAGS "-mconsole")
    endif()

    add_test(main openomf_test_main)

    message(STATUS "Development: Unit-tests are enabled")
else()
    message(STATUS "Development: Unit-tests are disabled")
endif()

include("cmake-scripts/BuildLanguages.cmake")

# Copy some resources to destination resources directory to ease development setup.
file(COPY resources/openomf.bk resources/gamecontrollerdb.txt DESTINATION resources)

# Installation
if(WIN32)
    # On windows, generate a flat directory structure under openomf/ subdir.
    # This way we have an "openomf/" root directory inside the zip file.
    install(TARGETS openomf RUNTIME DESTINATION openomf/ COMPONENT Binaries)
    install(FILES resources/openomf.bk resources/gamecontrollerdb.txt DESTINATION openomf/resources/ COMPONENT Data)
    install(DIRECTORY shaders/ DESTINATION openomf/shaders COMPONENT Data)
    install(FILES README.md BUILD.md LICENSE DESTINATION openomf/ COMPONENT Data)
else()
    # On unixy systems, follow standard.
    install(TARGETS openomf RUNTIME DESTINATION bin COMPONENT Binaries)
    install(FILES resources/openomf.bk resources/gamecontrollerdb.txt resources/icons/openomf.png
        DESTINATION share/games/openomf/
        COMPONENT Data
    )
    install(DIRECTORY shaders/ DESTINATION share/games/openomf/shaders COMPONENT Data)
endif()
