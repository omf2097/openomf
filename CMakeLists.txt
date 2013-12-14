cmake_minimum_required(VERSION 2.6)
project(OpenOMF C)

SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)
find_package(SDL2)
find_package(shadowdive)
find_package(enet)
find_package(dumb)
find_package(OpenGL)
find_package(OpenAL)
find_package(confuse)
find_package(GLEW)

# These flags are used for all builds
set(CORE_FLAGS "-Wall -std=c99 -Werror")

# When building with MingW, do not look for Libintl 
IF(MINGW)
    set(LIBINTL_INCLUDE_DIR "")
    set(LIBINTL_LIBRARIES "")
    set(CORE_FLAGS "${CORE_FLAGS} -static-libgcc")
ELSE(MINGW)
    find_package(Libintl)
ENDIF(MINGW)

# Options
OPTION(USE_OGGVORBIS "Add support for Ogg Vorbis audio" OFF)

# If we want to build support for vorbis streaming, find these too
IF(USE_OGGVORBIS)
    find_package(ogg)
    find_package(vorbis)
    set(CORE_FLAGS "${CORE_FLAGS} -DUSE_OGGVORBIS")
ENDIF(USE_OGGVORBIS)

# On Travis, compile in c11 mode
IF(TRAVIS)
    IF("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
        SET(CORE_FLAGS "-Wall -std=c11 -Werror")
    ENDIF()
ENDIF(TRAVIS)

# Build flags
set(CMAKE_C_FLAGS "${CORE_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CORE_FLAGS} -ggdb -DDEBUGMODE")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CORE_FLAGS} -g -O2")
set(CMAKE_C_FLAGS_RELEASE "${CORE_FLAGS} -O2 -s -march=native")
set(CMAKE_C_FLAGS_MINSIZEREL "${CORE_FLAGS} -Os -s -march=native")

# Don't show console on mingw in release builds
IF(MINGW)
    set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} -mwindows")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mwindows")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -mwindows")
ENDIF(MINGW)

# Testing stuff
enable_testing()
add_subdirectory(testing)

add_executable(openomf 
    src/utils/log.c
    src/utils/config.c
    src/utils/list.c
    src/utils/vector.c
    src/utils/hashmap.c
    src/utils/iterator.c
    src/utils/array.c
    src/utils/ringbuffer.c
    src/utils/vec.c
    src/utils/string.c
    src/video/video.c
    src/video/texture.c
    src/video/fbo.c
    src/video/shader.c
    src/video/shaderprogram.c
    src/video/rbo.c
    src/video/image.c
    src/video/color.c
    src/video/glextloader.c
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
    src/game/objects/har.c
    src/game/objects/scrap.c
    src/game/objects/projectile.c
    src/game/scenes/progressbar.c
    src/game/scenes/intro.c
    src/game/scenes/mainmenu.c
    src/game/scenes/credits.c
    src/game/scenes/arena.c
    src/game/scenes/melee.c
    src/game/scenes/vs.c
    src/game/scenes/mechlab.c
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
    src/controller/controller.c
    src/controller/keyboard.c
    src/controller/joystick.c
    src/controller/net_controller.c
    src/console/console.c
    src/main.c
    src/engine.c
)

set(COREINCS
    include
    ${SDL2_INCLUDE_DIR} 
    ${SHADOWDIVE_INCLUDE_DIR} 
    ${DUMB_INCLUDE_DIR} 
    ${OPENAL_INCLUDE_DIR} 
    ${OPENGL_INCLUDE_DIR}
    ${CONFUSE_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIR}
    ${LIBINTL_INCLUDE_DIR}
    ${ENET_INCLUDE_DIR}
)

set(CORELIBS 
    ${SDL2_LIBRARY} 
    ${SHADOWDIVE_LIBRARY} 
    ${DUMB_LIBRARY} 
    ${OPENGL_LIBRARY} 
    ${OPENAL_LIBRARY}
    ${CONFUSE_LIBRARY}
    ${GLEW_LIBRARY}
    ${LIBINTL_LIBRARIES}
    ${ENET_LIBRARY}
)

# If we support ogg vorbis, add library includes etc.
IF(USE_OGGVORBIS)
    set(COREINCS ${COREINCS} ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
    set(CORELIBS ${CORELIBS} ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY})
ENDIF(USE_OGGVORBIS)

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
target_link_libraries(openomf ${CORELIBS})
