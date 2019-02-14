cmake_minimum_required(VERSION 2.8)
project(OpenOMF C)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "6")
set(VERSION_PATCH "6")
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Options
OPTION(USE_TESTS "Build unittests" OFF)
OPTION(USE_TOOLS "Build tools" OFF)
OPTION(USE_OGGVORBIS "Add support for Ogg Vorbis audio" OFF)
OPTION(USE_DUMB "Use libdumb for module playback" OFF)
OPTION(USE_XMP "Use libxmp for module playback" ON)
OPTION(USE_OPENAL "Support OpenAL for audio playback" ON)
OPTION(USE_SANITIZERS "Enable Asan and Ubsan" OFF)

# These flags are used for all builds
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
find_package(SDL2)
find_package(enet)
find_package(confuse)
find_package(argtable2)
find_package(PNG)
find_package(ZLIB)

# Check functions and generate platform configuration file
CHECK_SYMBOL_EXISTS(strdup "string.h" HAVE_STD_STRDUP)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/include/platform.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/platform.h)

# If USE_DUMB flag is on, turn on libdumb
IF(USE_DUMB)
    find_package(dumb)
    add_definitions(-DUSE_DUMB)
    if(MINGW)
        add_definitions(-D_FILE_OFFSET_BITS=64)
    endif()
ENDIF()

# If XMP has been selected, attempt to find it
IF(USE_XMP)
    find_package(xmp)
    add_definitions(-DUSE_XMP)
ENDIF()

# Audio sink selection
IF(USE_OPENAL)
    find_package(OpenAL)
    add_definitions(-DUSE_OPENAL)
ELSE()
    MESSAGE(STATUS "Note! No audio sink selected; Music/sounds will not play.")
ENDIF()

# When building with MingW, do not look for Intl
# Also, use static libgcc when on mingw
IF(MINGW)
    set(LIBINTL_INCLUDE_DIR "")
    set(LIBINTL_LIBRARIES "")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
ELSE()
    find_package(Intl)
ENDIF()

# If we want to build support for vorbis streaming, find these too
IF(USE_OGGVORBIS)
    find_package(ogg)
    find_package(vorbis)
    add_definitions(-DUSE_OGGVORBIS)
ENDIF()

# If tests are enabled, find CUnit
IF(USE_TESTS)
    find_package(CUnit)
ENDIF()

# Only strip on GCC (clang does not appreciate)
IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,-s")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Wl,-s")
ENDIF()

# Find OpenOMF core sources
file(GLOB_RECURSE OPENOMF_SRC RELATIVE ${CMAKE_SOURCE_DIR} "src/*/*.c")

set(COREINCS
    include
    ${CMAKE_CURRENT_BINARY_DIR}/include/
    ${SDL2_INCLUDE_DIR}
    ${CONFUSE_INCLUDE_DIR}
    ${Intl_INCLUDE_DIR}
    ${ENET_INCLUDE_DIR}
    ${ARGTABLE2_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
)

set(CORELIBS
    ${SDL2_LIBRARY}
    ${CONFUSE_LIBRARY}
    ${Intl_LIBRARIES}
    ${ENET_LIBRARY}
    ${ARGTABLE2_LIBRARY}
    ${PNG_LIBRARY}
    ${ZLIB_LIBRARY}
)

# Handle module playback libraries
IF(USE_DUMB)
    set(CORELIBS ${CORELIBS} ${DUMB_LIBRARY})
    set(COREINCS ${COREINCS} ${DUMB_INCLUDE_DIR})
ENDIF()
IF(USE_XMP)
    set(CORELIBS ${CORELIBS} ${XMP_LIBRARY})
    set(COREINCS ${COREINCS} ${XMP_INCLUDE_DIR})
ENDIF()

# Audio sinks
IF(USE_OPENAL)
    set(CORELIBS ${CORELIBS} ${OPENAL_LIBRARY})
    set(COREINCS ${COREINCS} ${OPENAL_INCLUDE_DIR})
ENDIF()

# If we support ogg vorbis, add library includes etc.
IF(USE_OGGVORBIS)
    set(COREINCS ${COREINCS} ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
    set(CORELIBS ${CORELIBS} ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY})
ENDIF()

# MingW build should add mingw32 lib
IF(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
ENDIF()

# On windows, add winsock2 and winmm
IF(WIN32)
    set(CORELIBS ${CORELIBS} ws2_32 winmm)
ENDIF()

# On unix platforms, add libm (sometimes needed, it seems)
IF(UNIX)
    SET(CORELIBS ${CORELIBS} -lm)
ENDIF()

include_directories(${COREINCS})

# Build core sources first as an object library
# this can then be reused in tests and main executable to speed things up
add_library(openomf_core OBJECT ${OPENOMF_SRC})
set(CORELIBS openomf_core ${CORELIBS})

# Build the game binary
add_executable(openomf src/main.c src/engine.c)

# Build tools if requested
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

    target_link_libraries(bktool ${CORELIBS})
    target_link_libraries(aftool ${CORELIBS})
    target_link_libraries(soundtool ${CORELIBS})
    target_link_libraries(fonttool ${CORELIBS})
    target_link_libraries(languagetool ${CORELIBS} ${})
    target_link_libraries(omf_parse ${CORELIBS})
    target_link_libraries(afdiff ${CORELIBS})
    target_link_libraries(rectool ${CORELIBS})
    target_link_libraries(pictool ${CORELIBS})
    target_link_libraries(scoretool ${CORELIBS})
    target_link_libraries(trntool ${CORELIBS})
    target_link_libraries(altpaltool ${CORELIBS})
    target_link_libraries(chrtool ${CORELIBS})
    target_link_libraries(setuptool ${CORELIBS})
endif()

# Enable AddressSanitizer if requested (these libs need to be first on the list!)
IF(USE_SANITIZERS)
    set(CORELIBS asan ubsan ${CORELIBS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
    message(STATUS "DEBUG: Asan and Ubsan enabled")
ENDIF()

# Don't show console on mingw in release builds
IF(MINGW)
    IF(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set_target_properties(openomf PROPERTIES LINK_FLAGS "-mwindows")
    ENDIF(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
ENDIF(MINGW)
target_link_libraries(openomf ${CORELIBS})
set_property(TARGET openomf PROPERTY C_STANDARD 11)


# Testing stuff
IF(CUNIT_FOUND)
    enable_testing()

    include_directories(${CUNIT_INCLUDE_DIR} testing/ include/)
    SET(CORELIBS ${CORELIBS} ${CUNIT_LIBRARY})

    file(GLOB_RECURSE TEST_SRC RELATIVE ${CMAKE_SOURCE_DIR} "testing/*.c")

    add_executable(openomf_test_main ${TEST_SRC})

    target_link_libraries(openomf_test_main ${CORELIBS})
    set_property(TARGET openomf_test_main PROPERTY C_STANDARD 11)

    add_test(main openomf_test_main)
ENDIF(CUNIT_FOUND)

# Packaging
add_subdirectory(packaging)

# Installation
INSTALL(TARGETS openomf
    RUNTIME
    DESTINATION bin
    COMPONENT Binaries
)
INSTALL(FILES resources/openomf.bk resources/openomf_icon.png
    DESTINATION share/games/openomf/
    COMPONENT Data
)
