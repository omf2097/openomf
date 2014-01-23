cmake_minimum_required(VERSION 2.6)
project(OpenOMF C)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)
find_package(SDL2)
find_package(shadowdive)
find_package(enet)
find_package(dumb)
find_package(OpenAL)
find_package(confuse)

# These flags are used for all builds
set(CORE_FLAGS "-Wall")

# Check available compiler
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_C_COMPILER_VERSION VERSION_LESS 4.8)
        set(CORE_FLAGS "${CORE_FLAGS} -std=c99")
    elseif(CMAKE_C_COMPILER_VERSION VERSION_LESS 4.6)
        message(FATAL_ERROR "GCC version must be at least 4.6!")
    else()
        set(CORE_FLAGS "${CORE_FLAGS} -std=c11")
    endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CORE_FLAGS "${CORE_FLAGS} -std=c11")
    # On travis CMAKE_C_COMPILER_VERSION returns an empty string!
    if("${CMAKE_C_COMPILER_VERSION}" STREQUAL "")
        message(WARNING "Could not determine the version of your Clang compiler")
    else()
        if(CMAKE_C_COMPILER_VERSION VERSION_LESS 3.2)
            message(FATAL_ERROR "Clang version must be at least 3.2!")
        endif()
    endif()
else()
    set(CORE_FLAGS "${CORE_FLAGS} -std=c11")
    message(WARNING "Unsupported compiler!")
endif()

# When building with MingW, do not look for Libintl
# Also, use static libgcc when on mingw
IF(MINGW)
    set(LIBINTL_INCLUDE_DIR "")
    set(LIBINTL_LIBRARIES "")
    set(CORE_FLAGS "${CORE_FLAGS} -static-libgcc")
ELSE(MINGW)
    find_package(Libintl)
ENDIF(MINGW)

# Options
OPTION(USE_OGGVORBIS "Add support for Ogg Vorbis audio" OFF)
OPTION(USE_PNG "Add support for PNG screenshots" OFF)
OPTION(SERVER_ONLY "Do not build the game binary" OFF)

# If we want to build support for vorbis streaming, find these too
IF(USE_OGGVORBIS)
    find_package(ogg)
    find_package(vorbis)
    set(CORE_FLAGS "${CORE_FLAGS} -DUSE_OGGVORBIS")
ENDIF(USE_OGGVORBIS)

IF(USE_PNG)
    find_package(PNG)
    find_package(ZLIB)
    set(CORE_FLAGS "${CORE_FLAGS} -DUSE_PNG")
ENDIF(USE_PNG)

# Build flags
set(CMAKE_C_FLAGS "${CORE_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CORE_FLAGS} -ggdb -DDEBUGMODE -Werror")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CORE_FLAGS} -g -O2")
set(CMAKE_C_FLAGS_RELEASE "${CORE_FLAGS} -O2")
set(CMAKE_C_FLAGS_MINSIZEREL "${CORE_FLAGS} -Os")

# Only strip on GCC (clang does not appreciate)
if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,-s")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Wl,-s")
endif()

# Testing stuff
add_subdirectory(testing)

set(OPENOMF_SRC
    src/utils/log.c
    src/utils/config.c
    src/utils/list.c
    src/utils/vector.c
    src/utils/hashmap.c
    src/utils/iterator.c
    src/utils/array.c
    src/utils/ringbuffer.c
    src/utils/vec.c
    src/utils/str.c
    src/utils/random.c
    src/utils/miscmath.c
    src/video/video.c
    src/video/surface.c
    src/video/image.c
    src/video/tcache.c
    src/video/color.c
    src/video/video_hw.c
    src/video/video_soft.c
    src/audio/audio.c
    src/audio/music.c
    src/audio/sound.c
    src/audio/sink.c
    src/audio/stream.c
    src/audio/source.c
    src/audio/sinks/openal_sink.c
    src/audio/sinks/openal_stream.c
    src/audio/sources/dumb_source.c
    src/audio/sources/vorbis_source.c
    src/audio/sources/raw_source.c
    src/resources/ids.c
    src/resources/af.c
    src/resources/af_loader.c
    src/resources/af_move.c
    src/resources/bk.c
    src/resources/bk_info.c
    src/resources/bk_loader.c
    src/resources/palette.c
    src/resources/sprite.c
    src/resources/animation.c
    src/resources/sounds_loader.c
    src/game/protos/object.c
    src/game/protos/player.c
    src/game/protos/scene.c
    src/game/protos/intersect.c
    src/game/protos/object_specializer.c
    src/game/objects/har.c
    src/game/objects/scrap.c
    src/game/objects/projectile.c
    src/game/objects/hazard.c
    src/game/scenes/progressbar.c
    src/game/scenes/intro.c
    src/game/scenes/mainmenu.c
    src/game/scenes/credits.c
    src/game/scenes/arena.c
    src/game/scenes/melee.c
    src/game/scenes/vs.c
    src/game/scenes/mechlab.c
    src/game/scenes/newsroom.c
    src/game/objects/har.c
    src/game/text/text.c
    src/game/text/languages.c
    src/game/menu/menu.c
    src/game/menu/menu_background.c
    src/game/menu/textbutton.c
    src/game/menu/textselector.c
    src/game/menu/textslider.c
    src/game/menu/textinput.c
    src/game/menu/component.c
    src/game/settings.c
    src/game/score.c
    src/game/game_state.c
    src/game/game_player.c
    src/game/ticktimer.c
    src/game/serial.c
    src/controller/controller.c
    src/controller/keyboard.c
    src/controller/joystick.c
    src/controller/net_controller.c
    src/console/console.c
    src/console/console_cmd.c
    src/main.c
    src/engine.c
)

set(COREINCS
    include
    ${SDL2_INCLUDE_DIR} 
    ${SHADOWDIVE_INCLUDE_DIR} 
    ${DUMB_INCLUDE_DIR} 
    ${OPENAL_INCLUDE_DIR} 
    ${CONFUSE_INCLUDE_DIR}
    ${LIBINTL_INCLUDE_DIR}
    ${ENET_INCLUDE_DIR}
)

set(CORELIBS 
    ${SDL2_LIBRARY} 
    ${SHADOWDIVE_LIBRARY} 
    ${DUMB_LIBRARY} 
    ${OPENAL_LIBRARY}
    ${CONFUSE_LIBRARY}
    ${LIBINTL_LIBRARIES}
    ${ENET_LIBRARY}
)

# If we support ogg vorbis, add library includes etc.
IF(USE_OGGVORBIS)
    set(COREINCS ${COREINCS} ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
    set(CORELIBS ${CORELIBS} ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY})
ENDIF(USE_OGGVORBIS)

# If we support PNG, add library incs etc.
IF(USE_PNG)
    set(COREINCS ${COREINCS} ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
    set(CORELIBS ${CORELIBS} ${PNG_LIBRARY} ${ZLIB_LIBRARY})
ENDIF(USE_PNG)

# MingW build should add mingw32 lib
IF(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
ENDIF(MINGW)

# On windows, add winsock2 and winmm
IF(WIN32)
    set(CORELIBS ${CORELIBS} ws2_32 winmm)
ENDIF(WIN32)

# On unix platforms, add libm (sometimes needed, it seems)
IF(UNIX)
    SET(CORELIBS ${CORELIBS} -lm)
ENDIF(UNIX)

# On Travis, add more libs
IF(TRAVIS)
    SET(CORELIBS ${CORELIBS} -lasound -lpulse -lpulse-simple -lXi -lXrandr -lXss -lXinerama -lXcursor)
ENDIF(TRAVIS)

include_directories(${COREINCS})

# Build the server binary 
# XXX uncomment this later when we start working on the dedicated server
#add_executable(openomf_server ${OPENOMF_SRC})
#set_target_properties(openomf_server PROPERTIES COMPILE_DEFINITIONS "STANDALONE_SERVER=1")
#target_link_libraries(openomf_server ${CORELIBS})

# Build the game binary
IF(NOT SERVER_ONLY)
    add_executable(openomf ${OPENOMF_SRC})
    # Don't show console on mingw in release builds
    IF(MINGW)
        IF(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
            set_target_properties(openomf PROPERTIES LINK_FLAGS "-mwindows")
        ENDIF(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
    ENDIF(MINGW)
    target_link_libraries(openomf ${CORELIBS})
ENDIF(NOT SERVER_ONLY)

# Installation
INSTALL(TARGETS openomf
    RUNTIME DESTINATION bin
)
