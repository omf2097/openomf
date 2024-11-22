cmake_minimum_required(VERSION 3.16)
project(OpenOMF C)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "6")
set(VERSION_PATCH "6")
set(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Options
OPTION(USE_TESTS "Build unittests" OFF)
OPTION(USE_TOOLS "Build tools" OFF)
OPTION(USE_SANITIZERS "Enable Asan and Ubsan" OFF)
OPTION(USE_TIDY "Use clang-tidy for checks" OFF)
OPTION(USE_FORMAT "Use clang-format for checks" OFF)

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
    # we would probably like to pass /Wall instead of /W4, but there's a lot
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /permissive- /W4")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wformat -pedantic -Wvla")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb -DDEBUGMODE -Werror -fno-omit-frame-pointer")
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -g -O2 -fno-omit-frame-pointer -DNDEBUG")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O2 -DNDEBUG")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Os -DNDEBUG")
endif()
add_definitions(-DV_MAJOR=${VERSION_MAJOR} -DV_MINOR=${VERSION_MINOR} -DV_PATCH=${VERSION_PATCH})

# Enable AddressSanitizer if requested
if(USE_SANITIZERS)
    if(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        add_compile_options("-fsanitize=address")
        add_link_options("-fsanitize=address")
        message(STATUS "Development: Asan enabled. Ubsan is unsupported by MSVC.")
    else()
        add_compile_options("-fsanitize=address,undefined")
        add_link_options("-fsanitize=address,undefined")
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
endif()

if(WIN32)
    # prevent Windows.h from automatically defining as many macros
    add_definitions(-DWIN32_LEAN_AND_MEAN)

    if(NOT MINGW)
        # set source charset & runtime charset to utf-8
        add_compile_options("/utf-8")
    endif()
endif()

# System packages (hard dependencies)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(xmp)
find_package(Epoxy REQUIRED)
find_package(enet)
find_package(confuse)
find_package(argtable2)
find_package(Argtable3 CONFIG)
find_package(PNG)
find_package(miniupnpc)
find_package(natpmp)



# Check functions and generate platform configuration file
check_symbol_exists(strdup "string.h" HAVE_STD_STRDUP)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/platform.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/platform.h)

# If tests are enabled, find CUnit
if(USE_TESTS)
    find_package(CUnit REQUIRED)
endif()

# Only strip on GCC (clang does not appreciate)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,-s")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Wl,-s")
endif()

# Find OpenOMF core sources
file(GLOB_RECURSE OPENOMF_SRC
    LIST_DIRECTORIES OFF
    CONFIGURE_DEPENDS
    RELATIVE ${CMAKE_SOURCE_DIR}
    "src/*/*.c" "src/*/*.h"
)

# Remove all render plugin source code from OPENOMF_SRC
list(FILTER OPENOMF_SRC EXCLUDE REGEX "^src/video/renderers/.*/")
# and enable select render plugins
set(ENABLED_RENDER_PLUGINS opengl3 null)
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
endforeach()
set_source_files_properties("src/video/video.c" PROPERTIES COMPILE_DEFINITIONS "${VIDEO_C_DEFINES}")

set(COREINCS
    src
    ${CMAKE_CURRENT_BINARY_DIR}/src/
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${EPOXY_INCLUDE_DIR}
    ${CONFUSE_INCLUDE_DIR}
    ${XMP_INCLUDE_DIR}
    ${ENET_INCLUDE_DIR}
)

set(CORELIBS
    ${CONFUSE_LIBRARY}
    ${ENET_LIBRARY}
)

if(ARGTABLE2_FOUND)
  set(COREINCS ${COREINCS} ${ARGTABLE2_INCLUDE_DIR})
  set(CORELIBS ${CORELIBS} ${ARGTABLE2_LIBRARY})
  add_compile_definitions(ARGTABLE2_FOUND)
elseif(Argtable3_FOUND)
  set(CORELIBS ${CORELIBS} argtable3::argtable3)
  add_compile_definitions(ARGTABLE3_FOUND)
else()
  message(FATAL_ERROR "Neither argtable2 or argtable3 was found")
endif()

if(ARGTABLE2_FOUND)
  set(CORELIBS ${CORELIBS} ${ARGTABLE2_LIBRARY})
elseif(ARGTABLE3_FOUND)
  set(CORELIBS ${CORELIBS} ${ARGTABLE3_LIBRARY})
endif()

if(MINIUPNPC_FOUND)
    set(CORELIBS ${CORELIBS} ${MINIUPNPC_LIBRARY})
    set(COREINCS ${COREINCS} ${MINIUPNPC_INCLUDE_DIR})
    add_compile_definitions(MINIUPNPC_FOUND)
endif()

if(LIBNATPMP_FOUND)
    set(CORELIBS ${CORELIBS} ${LIBNATPMP_LIBRARY})
    set(COREINCS ${COREINCS} ${LIBNATPMP_INCLUDE_DIR})
    add_compile_definitions(NATPMP_FOUND)
endif()

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

# Build core sources first as an object library
# this can then be reused in tests and main executable to speed things up
add_library(openomf_core OBJECT ${OPENOMF_SRC})
target_link_libraries(openomf_core PRIVATE
    ${XMP_LIBRARY}
    PNG::PNG
)
omf_target_precompile_headers(openomf_core PUBLIC
    "<SDL.h>"
    "<enet/enet.h>"
    "<epoxy/gl.h>"
)
set(CORELIBS openomf_core ${CORELIBS})

if(WIN32 AND NOT MINGW)
    omf_target_precompile_headers(openomf_core PUBLIC
        "<windows.h>"
    )
endif()

# Set icon for windows executable
if(WIN32)
    SET(ICON_RESOURCE "resources/icons/openomf.rc")
endif()

# Build the game binary
add_executable(openomf src/main.c src/engine.c ${ICON_RESOURCE})
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT openomf)

# This is used to copy shader files from source directory to the binary output directory during compile.
add_custom_target(
    copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
)
add_dependencies(openomf copy_shaders)

# Build tools if requested
set(TOOL_TARGET_NAMES)
# always build languagetool, for BuildLanguages.cmake
add_executable(languagetool tools/languagetool/main.c)
list(APPEND TOOL_TARGET_NAMES languagetool)
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
    include(ClangFormat)
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
target_link_libraries(openomf ${CORELIBS})
target_link_libraries(openomf SDL2::Main SDL2::Mixer Epoxy::Main)
foreach(TARGET ${TOOL_TARGET_NAMES})
    target_link_libraries(${TARGET} ${CORELIBS})
    target_link_libraries(${TARGET} SDL2::Main SDL2::Mixer Epoxy::Main)
endforeach()

# Testing stuff
if(CUNIT_FOUND)
    enable_testing()

    include_directories(${CUNIT_INCLUDE_DIR} testing/ src/)
    SET(CORELIBS ${CORELIBS} ${CUNIT_LIBRARY})

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

    target_link_libraries(openomf_test_main ${CORELIBS} SDL2::Main SDL2::Mixer Epoxy::Main)

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
