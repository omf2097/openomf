cmake_minimum_required(VERSION 2.6)
project(omf2097-tools C)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

option(USE_SQLITE "Use sqlite3 (enables stringtool)" ON)

set(CMAKE_C_FLAGS "-Wall -std=c99")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -Werror")
set(CMAKE_C_FLAGS_RELEASE "-O2")

find_package(SDL2)
find_package(argtable2)
find_package(shadowdive)
find_package(PNG)

if(USE_SQLITE)
    find_package(sqlite)
    add_executable(stringtool src/stringtool/main.c)
endif()

add_executable(bktool src/bktool/main.c src/shared/animation_misc.c src/shared/conversions.c)
add_executable(aftool src/aftool/main.c src/shared/animation_misc.c src/shared/conversions.c)
add_executable(soundtool src/soundtool/main.c)
add_executable(fonttool src/fonttool/main.c)
add_executable(languagetool src/languagetool/main.c)
add_executable(omf_parse src/stringparser/main.c)
add_executable(afdiff src/afdiff/main.c)
add_executable(rectool src/rectool/main.c src/shared/pilot.c)
add_executable(pictool src/pictool/main.c)
add_executable(scoretool src/scoretool/main.c)
add_executable(trntool src/trntool/main.c src/shared/pilot.c)
add_executable(altpaltool src/altpaltool/main.c)
add_executable(chrtool src/chrtool/main.c src/shared/pilot.c)
add_executable(setuptool src/setuptool/main.c src/shared/pilot.c)

set(CORELIBS ${SHADOWDIVE_LIBRARY} ${ARGTABLE2_LIBRARY} ${PNG_LIBRARY} ${ZLIB_LIBRARY})

# Fix compilation issues on mingw
if(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
endif()

# Fix compilation issues on unixy systems (missing math lib)
if(UNIX)
    SET(CORELIBS ${CORELIBS} -lm)
endif(UNIX)

include_directories(
    ${SDL2_INCLUDE_DIR}
    ${PNG_INCLUDE_DIR}
    ${ARGTABLE2_INCLUDE_DIR}
    ${SHADOWDIVE_INCLUDE_DIR}
)

# Set correct libs for stringtool
if(USE_SQLITE)
    include_directories(${SQLITE_INCLUDE_DIR})
    target_link_libraries(stringtool ${CORELIBS} ${SHADOWDIVE_LIBRARY} ${SQLITE_LIBRARY})
endif()

target_link_libraries(bktool ${CORELIBS} ${SDL2_LIBRARY})
target_link_libraries(aftool ${CORELIBS} ${SDL2_LIBRARY})
target_link_libraries(soundtool ${CORELIBS} ${SDL2_LIBRARY})
target_link_libraries(fonttool ${CORELIBS} ${SDL2_LIBRARY})
target_link_libraries(languagetool ${CORELIBS} ${SDL2_LIBRARY})
target_link_libraries(omf_parse ${CORELIBS})
target_link_libraries(afdiff ${CORELIBS})
target_link_libraries(rectool ${CORELIBS})
target_link_libraries(pictool ${CORELIBS})
target_link_libraries(scoretool ${CORELIBS})
target_link_libraries(trntool ${CORELIBS})
target_link_libraries(altpaltool ${CORELIBS})
target_link_libraries(chrtool ${CORELIBS})
target_link_libraries(setuptool ${CORELIBS})

install(
    TARGETS
        aftool
        bktool
        afdiff
        omf_parse
        soundtool
        fonttool
        languagetool
        rectool
        pictool
        scoretool
        trntool
        altpaltool
        chrtool
        setuptool
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
