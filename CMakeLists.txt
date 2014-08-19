cmake_minimum_required(VERSION 2.6)
project(ShadowDive C)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

set(CMAKE_C_FLAGS "-Wall -std=c99")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -DDEBUGMODE -fno-omit-frame-pointer -pedantic -Werror")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2 -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os")
add_definitions(-DSD_USE_INTERNAL)

OPTION(BUILD_TESTS "Build unittests" OFF)
OPTION(BUILD_OLD_TESTS "Build old test programs" OFF)

include(${CMAKE_MODULE_PATH}/CMakeLists-emscripten.cmake)
link_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(include/)

IF(BUILD_TESTS)
    add_subdirectory(testing)
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
    src/stringparser.c
    src/vga_image.c
    src/language.c
    src/tournament.c
    src/pic.c
    src/score.c
    src/chr.c
    src/pilot.c
    src/rec.c
    src/internal/reader.c
    src/internal/memreader.c
    src/internal/writer.c
    src/internal/helpers.c
)

IF(BUILD_OLD_TESTS)
    #add_executable(test_main test/test_main.c)
    #add_executable(test_roundtrip test/roundtrip.c)
    add_executable(test_font test/test_fonts.c)
    #add_executable(test_stringparser test/test_stringparser.c)
    #add_executable(test_language test/test_language.c)
    add_executable(test_altpals test/test_altpals.c)
    add_executable(test_tourn test/test_tourn.c test/helpers.c)
    add_executable(test_pic test/test_pic.c)
    #add_executable(test_score test/test_score.c)
    add_executable(test_chr test/test_chr.c)
    add_executable(test_rec test/test_rec.c test/helpers.c)

    #target_link_libraries(test_main shadowdive)
    #target_link_libraries(test_roundtrip shadowdive)
    target_link_libraries(test_font shadowdive)
    #target_link_libraries(test_stringparser shadowdive)
    #target_link_libraries(test_language shadowdive)
    target_link_libraries(test_altpals shadowdive)
    target_link_libraries(test_tourn shadowdive)
    target_link_libraries(test_pic shadowdive)
    #target_link_libraries(test_score shadowdive)
    target_link_libraries(test_chr shadowdive)
    target_link_libraries(test_rec shadowdive)
ENDIF()

FILE(GLOB H_FILES "include/shadowdive/*.h")
INSTALL(FILES ${H_FILES} DESTINATION include/shadowdive/)
INSTALL(TARGETS shadowdive
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
