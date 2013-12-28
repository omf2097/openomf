cmake_minimum_required(VERSION 2.6)
project(omf2097-tools C)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)
find_package(SDL2)
find_package(argtable2)
find_package(shadowdive)
find_package(dumb)
find_package(PNG)
find_package(sqlite)

set(COMMON_FLAGS "-Wall -std=c99 -Werror")
set(CMAKE_C_FLAGS "${COMMON_FLAGS} -ggdb")
set(CMAKE_C_FLAGS_DEBUG "${COMMON_FLAGS} -ggdb")
set(CMAKE_C_FLAGS_RELEASE "${COMMON_FLAGS} -O2")

add_executable(bktool src/bktool/main.c src/shared/animation_misc.c src/shared/conversions.c)
add_executable(aftool src/aftool/main.c src/shared/animation_misc.c src/shared/conversions.c)
add_executable(bkhtmlprinter src/bkhtmlprinter/main.c)
add_executable(afhtmlprinter src/afhtmlprinter/main.c)
add_executable(soundtool src/soundtool/main.c)
add_executable(musictool src/musictool/main.c)
add_executable(fonttool src/fonttool/main.c)
add_executable(languagetool src/languagetool/main.c)

set(CORELIBS ${ARGTABLE2_LIBRARY})
IF(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
ENDIF(MINGW)

include_directories(
    ${SDL2_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${ARGTABLE2_INCLUDE_DIR}
    ${SHADOWDIVE_INCLUDE_DIR}
    ${DUMB_INCLUDE_DIR}
) 

IF(SQLITE_FOUND)
    include_directories(
        ${SQLITE_INCLUDE_DIR}
    )
    add_executable(stringtool src/stringtool/main.c)
    target_link_libraries(stringtool ${CORELIBS} ${SHADOWDIVE_LIBRARY} ${SQLITE_LIBRARY})
ENDIF(SQLITE_FOUND)

target_link_libraries(bktool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(aftool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(bkhtmlprinter ${CORELIBS} ${PNG_LIBRARY} ${ZLIB_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(afhtmlprinter ${CORELIBS} ${PNG_LIBRARY} ${ZLIB_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(soundtool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(musictool ${CORELIBS} ${SDL2_LIBRARY} ${DUMB_LIBRARY})
target_link_libraries(fonttool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(languagetool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})


INSTALL(TARGETS bktool afhtmlprinter bkhtmlprinter aftool soundtool musictool fonttool languagetool
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
