cmake_minimum_required(VERSION 2.6)
project(ShadowDive C)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

set(CMAKE_C_FLAGS "-Wall -std=c99")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -DDEBUGMODE -fno-omit-frame-pointer -pedantic -Werror")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2 -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
add_definitions(-DSD_USE_INTERNAL)

OPTION(BUILD_TESTS "Build unittests" OFF)
OPTION(USE_PNG "Use libpng for importers and exporters" ON)

include(${CMAKE_MODULE_PATH}/CMakeLists-emscripten.cmake)
link_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(include/)

IF(USE_PNG)
    find_package(PNG)
    add_definitions(-DUSE_PNG)
ENDIF()

IF(BUILD_TESTS)
    add_subdirectory(testing)
ENDIF()

# zlib may be required if using tests
IF(BUILD_TESTS AND USE_PNG)
    find_package(ZLIB)
ENDIF()

add_library(shadowdive
    src/af.c
    src/altpal.c
    src/animation.c
    src/bk.c
    src/bkanim.c
    src/error.c
    src/fonts.c
    src/move.c
    src/palette.c
    src/rgba_image.c
    src/sounds.c
    src/sprite.c
    src/vga_image.c
    src/language.c
    src/tournament.c
    src/pic.c
    src/score.c
    src/chr.c
    src/setup.c
    src/pilot.c
    src/rec.c
    src/taglist.c
    src/taglist_helpers.c
    src/script.c
    src/internal/reader.c
    src/internal/memreader.c
    src/internal/memwriter.c
    src/internal/writer.c
    src/internal/array.c
)

IF(USE_PNG)
    include_directories(${PNG_INCLUDE_DIR})
    target_link_libraries(shadowdive ${PNG_LIBRARY})
ENDIF()

FILE(GLOB H_FILES "include/shadowdive/*.h")
INSTALL(FILES ${H_FILES} DESTINATION include/shadowdive/)
INSTALL(TARGETS shadowdive
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
