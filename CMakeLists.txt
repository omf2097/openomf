cmake_minimum_required(VERSION 2.6)
project(OpenOMF C)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)
find_package(SDL2)
find_package(shadowdive)

set(CMAKE_C_FLAGS "-Wall -std=c99 -ggdb -Werror")
set(CMAKE_C_FLAGS_DEBUG "-Wall -std=c99 -ggdb -Werror")
set(CMAKE_C_FLAGS_RELEASE "-Wall -std=c99 -O2 -march=i686 -Werror")

add_executable(openomf 
    src/main.c
)

include_directories(${SDL2_INCLUDE_DIR} ${SHADOWDIVE_INCLUDE_DIR} include)
target_link_libraries(openomf ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})