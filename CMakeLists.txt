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
find_package(Libintl)

IF(MINGW)
    set(CMAKE_C_FLAGS "-Wall -std=c99 -Werror")
ELSE(MINGW)
    set(CMAKE_C_FLAGS "-Wall -std=c99 -Werror")
ENDIF(MINGW)

set(CMAKE_C_FLAGS_DEBUG "-ggdb -DDEBUGMODE")
set(CMAKE_C_FLAGS_RELEASE "-O2 -march=native")

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
    src/video/texturelist.c
    src/audio/audio.c
    src/audio/music.c
    src/audio/sound.c
    src/audio/stream.c
    src/audio/soundloader.c
    src/resources/af.c
    src/resources/af_loader.c
    src/resources/af_move.c
    src/resources/bk.c
    src/resources/bk_info.c
    src/resources/bk_loader.c
    src/resources/palette.c
    src/resources/sprite.c
    src/resources/animation.c
    src/game/protos/object.c
    src/game/protos/player.c
    src/game/protos/scene.c
    src/game/objects/har.c
    src/game/scenes/progressbar.c
    src/game/scenes/intro.c
    src/game/scenes/mainmenu.c
    src/game/scenes/credits.c
    src/game/scenes/arena.c
    src/game/scenes/melee.c
    src/game/scenes/vs.c
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
    src/controller/controller.c
    src/controller/keyboard.c
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

IF(MINGW)
    set(CORELIBS mingw32 ${CORELIBS} ws2_32 winmm)
ENDIF(MINGW)

IF(UNIX)
    SET(CORELIBS ${CORELIBS} -lm)
ENDIF(UNIX)

include_directories(${COREINCS})
target_link_libraries(openomf ${CORELIBS})
