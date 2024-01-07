cmake_minimum_required(VERSION 3.7.2)
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

# These flags are used for all builds
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -DDEBUGMODE -Werror -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2 -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
add_definitions(-DV_MAJOR=${VERSION_MAJOR} -DV_MINOR=${VERSION_MINOR} -DV_PATCH=${VERSION_PATCH})

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
    add_definitions(-DSHA1_HASH="${SHA1_HASH}")
endif()

# System packages (hard dependencies)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(xmp)
find_package(enet)
find_package(confuse)
find_package(argtable2)
find_package(PNG)
find_package(ZLIB)

# Check functions and generate platform configuration file
check_symbol_exists(strdup "string.h" HAVE_STD_STRDUP)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/platform.h.in ${CMAKE_CURRENT_BINARY_DIR}/src/platform.h)

# When building with MingW, do not look for Intl
if(MINGW)
    set(LIBINTL_INCLUDE_DIR "")
    set(LIBINTL_LIBRARIES "")
else()
    find_package(Intl)
endif()

# If tests are enabled, find CUnit
if(USE_TESTS)
    find_package(CUnit)
endif()

# Only strip on GCC (clang does not appreciate)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,-s")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Wl,-s")
endif()

# Find OpenOMF core sources
file(GLOB_RECURSE OPENOMF_SRC RELATIVE ${CMAKE_SOURCE_DIR} "src/*/*.c")

set(COREINCS
    src
    ${CMAKE_CURRENT_BINARY_DIR}/src/
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${CONFUSE_INCLUDE_DIR}
    ${Intl_INCLUDE_DIR}
    ${XMP_INCLUDE_DIR}
    ${ENET_INCLUDE_DIR}
    ${ARGTABLE2_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
)

set(CORELIBS
    ${CONFUSE_LIBRARY}
    ${Intl_LIBRARIES}
    ${XMP_LIBRARY}
    ${ENET_LIBRARY}
    ${ARGTABLE2_LIBRARY}
    ${PNG_LIBRARY}
    ${ZLIB_LIBRARY}
)

# MingW build should add mingw32 lib
if(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
endif()

# On windows, add winsock2 and winmm
if(WIN32)
    set(CORELIBS ${CORELIBS} ws2_32 winmm)
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
set(CORELIBS openomf_core ${CORELIBS})

# Set icon for windows executable
if(WIN32)
    SET(ICON_RESOURCE "resources/icons/openomf.rc")
endif()

# Build the game binary
add_executable(openomf src/main.c src/engine.c ${ICON_RESOURCE})

# Build tools if requested
set(TOOL_TARGET_NAMES)
if(USE_TOOLS)
    add_executable(bktool tools/bktool/main.c
                          tools/shared/animation_misc.c
                          tools/shared/conversions.c)
    add_executable(aftool tools/aftool/main.c
                          tools/shared/animation_misc.c
                          tools/shared/conversions.c)
    add_executable(soundtool tools/soundtool/main.c)
    add_executable(fonttool tools/fonttool/main.c)
    add_executable(languagetool tools/languagetool/main.c)
    add_executable(omf_parse tools/stringparser/main.c)
    add_executable(afdiff tools/afdiff/main.c)
    add_executable(rectool tools/rectool/main.c tools/shared/pilot.c)
    add_executable(pictool tools/pictool/main.c)
    add_executable(scoretool tools/scoretool/main.c)
    add_executable(trntool tools/trntool/main.c tools/shared/pilot.c)
    add_executable(altpaltool tools/altpaltool/main.c)
    add_executable(chrtool tools/chrtool/main.c tools/shared/pilot.c)
    add_executable(setuptool tools/setuptool/main.c tools/shared/pilot.c)

    list(APPEND TOOL_TARGET_NAMES
        bktool
        aftool
        soundtool
        fonttool
        languagetool
        omf_parse
        afdiff
        rectool
        pictool
        scoretool
        trntool
        altpaltool
        chrtool
        setuptool
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
    file(
        GLOB_RECURSE
        SRC_FILES
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

# Enable AddressSanitizer if requested (these libs need to be first on the list!)
if(USE_SANITIZERS)
    set(CORELIBS asan ubsan ${CORELIBS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
    message(STATUS "Development: Asan and Ubsan enabled")
else()
    message(STATUS "Development: Asan and Ubsan disabled")
endif()

if(MINGW)
    # Don't show console on mingw in release builds (but enable if debug mode)
    if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set_target_properties(openomf PROPERTIES LINK_FLAGS "-mwindows")
    else()
        set_target_properties(openomf PROPERTIES LINK_FLAGS "-mconsole")
    endif()

    # Make sure console apps are always in terminal output mode.
    foreach(TARGET ${TOOL_TARGET_NAMES})
        set_target_properties(${TARGET} PROPERTIES LINK_FLAGS "-mconsole")
    endforeach()

    # Use static libgcc when on mingw
    target_link_options(openomf PRIVATE -static-libgcc)
    foreach(TARGET ${TOOL_TARGET_NAMES})
        target_link_options(${TARGET} PRIVATE -static-libgcc)
    endforeach()
endif()

# Make sure libraries are linked
target_link_libraries(openomf ${CORELIBS})
target_link_libraries(openomf SDL2::Main SDL2::Mixer)
foreach(TARGET ${TOOL_TARGET_NAMES})
    target_link_libraries(${TARGET} ${CORELIBS} SDL2::Main SDL2::Mixer)
endforeach()

# Testing stuff
if(CUNIT_FOUND)
    enable_testing()

    include_directories(${CUNIT_INCLUDE_DIR} testing/ src/)
    SET(CORELIBS ${CORELIBS} ${CUNIT_LIBRARY})

    file(GLOB_RECURSE TEST_SRC RELATIVE ${CMAKE_SOURCE_DIR} "testing/*.c")

    add_executable(openomf_test_main ${TEST_SRC})

    # This makes loading test resources possible
    target_compile_definitions(openomf_test_main PRIVATE
                               TESTS_ROOT_DIR="${CMAKE_SOURCE_DIR}/testing")

    target_link_libraries(openomf_test_main ${CORELIBS})

    if(MINGW)
        # Always build as a console executable with mingw
        set_target_properties(openomf_test_main PROPERTIES LINK_FLAGS "-mconsole")
    endif()

    add_test(main openomf_test_main)

    message(STATUS "Development: Unit-tests are enabled")
else()
    message(STATUS "Development: Unit-tests are disabled")
endif()

# Packaging
add_subdirectory(packaging)

# Installation
if(WIN32)
    # On windows, generate a flat directory structure under openomf/ subdir.
    # This way we have an "openomf/" root directory inside the zip file.
    install(TARGETS openomf RUNTIME DESTINATION openomf/ COMPONENT Binaries)
    install(FILES resources/openomf.bk DESTINATION openomf/resources/ COMPONENT Data)
    install(FILES README.md INSTALL.md LICENSE DESTINATION openomf/ COMPONENT Data)
else()
    # On unixy systems, follow standard.
    install(TARGETS openomf RUNTIME DESTINATION bin COMPONENT Binaries)
    install(FILES resources/openomf.bk resources/icons/openomf.png
        DESTINATION share/games/openomf/
        COMPONENT Data
    )
endif()
