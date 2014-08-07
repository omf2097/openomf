cmake_minimum_required(VERSION 2.6)
project(omf2097-tools C)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

OPTION(USE_MODPLUG "Use libmodplug for music playback" OFF)
OPTION(USE_DUMB "Use libdumb for music playback" ON)
OPTION(USE_SQLITE "Use sqlite3 (enables stringtool)" ON)

set(CMAKE_C_FLAGS "-Wall -std=c99")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -Werror")
set(CMAKE_C_FLAGS_RELEASE "-O2")

find_package(SDL2)
find_package(argtable2)
find_package(shadowdive)
find_package(PNG)

IF(USE_SQLITE)
    find_package(sqlite)
    add_executable(stringtool src/stringtool/main.c)
ENDIF()

IF(USE_DUMB)
    find_package(dumb)
    add_definitions(-DUSE_DUMB)
    add_executable(musictool src/musictool/main.c)
ELSEIF(USE_MODPLUG)
    find_package(modplug)
    add_definitions(-DUSE_MODPLUG)
    add_executable(musictool src/musictool/main.c)
ENDIF()

add_executable(bktool src/bktool/main.c src/shared/animation_misc.c src/shared/conversions.c)
add_executable(aftool src/aftool/main.c src/shared/animation_misc.c src/shared/conversions.c)
add_executable(bkhtmlprinter src/bkhtmlprinter/main.c)
add_executable(afhtmlprinter src/afhtmlprinter/main.c)
add_executable(soundtool src/soundtool/main.c)
add_executable(fonttool src/fonttool/main.c)
add_executable(languagetool src/languagetool/main.c)
add_executable(omf_parse src/stringparser/main.c)
add_executable(afdiff src/afdiff/main.c)

set(CORELIBS ${ARGTABLE2_LIBRARY})
IF(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
ENDIF()

include_directories(
    ${SDL2_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${ARGTABLE2_INCLUDE_DIR}
    ${SHADOWDIVE_INCLUDE_DIR}
) 

# Set correct libs for stringtool
IF(USE_SQLITE)
    include_directories(${SQLITE_INCLUDE_DIR})
    target_link_libraries(stringtool ${CORELIBS} ${SHADOWDIVE_LIBRARY} ${SQLITE_LIBRARY})
ENDIF()

# Set correct libs for musictool
IF(USE_DUMB)
    include_directories(${DUMB_INCLUDE_DIR})
    target_link_libraries(musictool ${CORELIBS} ${SDL2_LIBRARY} ${DUMB_LIBRARY})
ELSEIF(USE_MODPLUG)
    include_directories(${MODPLUG_INCLUDE_DIR})
    target_link_libraries(musictool ${CORELIBS} ${SDL2_LIBRARY} ${MODPLUG_LIBRARY})
ENDIF()

target_link_libraries(bktool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(aftool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(bkhtmlprinter ${CORELIBS} ${PNG_LIBRARY} ${ZLIB_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(afhtmlprinter ${CORELIBS} ${PNG_LIBRARY} ${ZLIB_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(soundtool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(fonttool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(languagetool ${CORELIBS} ${SDL2_LIBRARY} ${SHADOWDIVE_LIBRARY})
target_link_libraries(omf_parse ${CORELIBS} ${SHADOWDIVE_LIBRARY})
target_link_libraries(afdiff ${CORELIBS} ${SHADOWDIVE_LIBRARY})

INSTALL(TARGETS aftool bktool afdiff omf_parse bkhtmlprinter afhtmlprinter soundtool fonttool languagetool
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
