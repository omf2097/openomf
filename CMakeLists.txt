cmake_minimum_required(VERSION 2.6)
project(omf2097-tools)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)
find_package(SDL2)
find_package(argtable2)
find_package(shadowdive)

set(CMAKE_C_FLAGS "-Wall -std=c99")
set(CMAKE_C_FLAGS_DEBUG "-ggdb")
set(CMAKE_C_FLAGS_RELEASE "-O2 -march=i686")
add_subdirectory(src)

add_executable(bktool src/bktool/main.c)
add_executable(aftool src/aftool/main.c)
add_executable(soundtool src/soundtool/main.c)

set(CORELIBS ${SDL2_LIBRARY} ${ARGTABLE2_LIBRARY} ${SHADOWDIVE_LIBRARY})
IF(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
ENDIF(MINGW)

include_directories(${SDL2_INCLUDE_DIR} ${ARGTABLE2_INCLUDE_DIR} ${SHADOWDIVE_INCLUDE_DIR})

target_link_libraries(bktool ${CORELIBS})
target_link_libraries(aftool ${CORELIBS})
target_link_libraries(soundtool ${CORELIBS})