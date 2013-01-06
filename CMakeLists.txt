cmake_minimum_required(VERSION 2.6)
project(ShadowDive C)

set(CMAKE_C_FLAGS "-Wall -std=gnu99 -ggdb -Werror")
set(CMAKE_C_FLAGS_DEBUG "-Wall -std=gnu99 -ggdb -Werror")
set(CMAKE_C_FLAGS_RELEASE "-Wall -std=gnu99 -O2 -march=i686 -Werror")

link_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(include/)

add_library(shadowdive
    src/af.c
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
    src/sprite_image.c
    src/stringparser.c
    src/vga_image.c
    src/language.c
    src/internal/reader.c
    src/internal/writer.c
    src/internal/helpers.c
)

add_executable(test_main test/test_main.c)
add_executable(test_sounds test/sounds_main.c)
add_executable(test_roundtrip test/roundtrip.c)
add_executable(test_font test/test_fonts.c)
add_executable(test_stringparser test/test_stringparser.c)
add_executable(test_language test/test_language.c)

target_link_libraries(test_main shadowdive)
target_link_libraries(test_sounds shadowdive)
target_link_libraries(test_roundtrip shadowdive)
target_link_libraries(test_font shadowdive)
target_link_libraries(test_stringparser shadowdive)
target_link_libraries(test_language shadowdive)

FILE(GLOB H_FILES "include/shadowdive/*.h")
INSTALL(FILES ${H_FILES} DESTINATION include/shadowdive/)
INSTALL(TARGETS shadowdive
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
