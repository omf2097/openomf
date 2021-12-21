cmake_minimum_required(VERSION 3.7)
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
find_package(SDL2 REQUIRED)
find_package(enet)
find_package(confuse)
find_package(argtable2)
find_package(PNG)
find_package(ZLIB)

# Check functions and generate platform configuration file
check_symbol_exists(strdup "string.h" HAVE_STD_STRDUP)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/platform.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/platform.h)

# If USE_DUMB flag is on, turn on libdumb
if(USE_DUMB)
    find_package(dumb)
    add_definitions(-DUSE_DUMB)
    if(MINGW)
        add_definitions(-D_FILE_OFFSET_BITS=64)
    endif()
endif()

# If XMP has been selected, attempt to find it
if(USE_XMP)
    find_package(xmp)
    add_definitions(-DUSE_XMP)
endif()

# Audio sink selection
if(USE_OPENAL)
    find_package(OpenAL)
    add_definitions(-DUSE_OPENAL)
else()
    message(STATUS "Note! No audio sink selected; Music/sounds will not play.")
endif()

# When building with MingW, do not look for Intl
# Also, use static libgcc when on mingw
if(MINGW)
    set(LIBINTL_INCLUDE_DIR "")
    set(LIBINTL_LIBRARIES "")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
else()
    find_package(Intl)
endif()

# If we want to build support for vorbis streaming, find these too
if(USE_OGGVORBIS)
    find_package(ogg)
    find_package(vorbis)
    add_definitions(-DUSE_OGGVORBIS)
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
    include
    ${CMAKE_CURRENT_BINARY_DIR}/include/
    ${SDL2_INCLUDE_DIRS}
    ${CONFUSE_INCLUDE_DIR}
    ${Intl_INCLUDE_DIR}
    ${ENET_INCLUDE_DIR}
    ${ARGTABLE2_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIR}
)

set(CORELIBS
    ${SDL2_LIBRARIES}
    ${CONFUSE_LIBRARY}
    ${Intl_LIBRARIES}
    ${ENET_LIBRARY}
    ${ARGTABLE2_LIBRARY}
    ${PNG_LIBRARY}
    ${ZLIB_LIBRARY}
)

# Handle module playback libraries
if(USE_DUMB)
    set(CORELIBS ${CORELIBS} ${DUMB_LIBRARY})
    set(COREINCS ${COREINCS} ${DUMB_INCLUDE_DIR})
endif()
if(USE_XMP)
    set(CORELIBS ${CORELIBS} ${XMP_LIBRARY})
    set(COREINCS ${COREINCS} ${XMP_INCLUDE_DIR})
endif()

# Audio sinks
if(USE_OPENAL)
    set(CORELIBS ${CORELIBS} ${OPENAL_LIBRARY})
    set(COREINCS ${COREINCS} ${OPENAL_INCLUDE_DIR})
endif()

# If we support ogg vorbis, add library includes etc.
if(USE_OGGVORBIS)
    set(COREINCS ${COREINCS} ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
    set(CORELIBS ${CORELIBS} ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY})
endif()

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
    target_link_libraries(languagetool ${CORELIBS})
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
if(USE_SANITIZERS)
    set(CORELIBS asan ubsan ${CORELIBS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
    message(STATUS "DEBUG: Asan and Ubsan enabled")
endif()

# Don't show console on mingw in release builds
if(MINGW)
    if(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
        set_target_properties(openomf PROPERTIES LINK_FLAGS "-mwindows")
    endif(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
endif()

# Link options for main binary
target_link_libraries(openomf ${CORELIBS})
set_property(TARGET openomf PROPERTY C_STANDARD 11)

# Testing stuff
if(CUNIT_FOUND)
    enable_testing()

    include_directories(${CUNIT_INCLUDE_DIR} testing/ include/)
    SET(CORELIBS ${CORELIBS} ${CUNIT_LIBRARY})

    file(GLOB_RECURSE TEST_SRC RELATIVE ${CMAKE_SOURCE_DIR} "testing/*.c")

    add_executable(openomf_test_main ${TEST_SRC})

    # This makes loading test resources possible
    target_compile_definitions(openomf_test_main PRIVATE
                               TESTS_ROOT_DIR="${CMAKE_SOURCE_DIR}/testing")

    target_link_libraries(openomf_test_main ${CORELIBS})
    set_property(TARGET openomf_test_main PROPERTY C_STANDARD 11)

    add_test(main openomf_test_main)
endif()

# Packaging
add_subdirectory(packaging)

# Installation
if(WIN32)
    # On windows, generate a flat directory structure under openomf/ subdir.
    # This way we have an "openomf/" root directory inside the zip file.
    install(TARGETS openomf RUNTIME DESTINATION openomf/ COMPONENT Binaries)
    install(FILES resources/openomf.bk DESTINATION openomf/resources/ COMPONENT Data)
    install(FILES README.md INSTALL.md LICENSE resources/icons/openomf.ico resources/icons/openomf.png
        DESTINATION openomf/
        COMPONENT Data
    )
else()
    # On unixy systems, follow standard.
    install(TARGETS openomf RUNTIME DESTINATION bin COMPONENT Binaries)
    install(FILES resources/openomf.bk resources/icons/openomf.png
        DESTINATION share/games/openomf/
        COMPONENT Data
    )
endif()
