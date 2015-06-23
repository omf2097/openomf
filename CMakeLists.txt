cmake_minimum_required(VERSION 2.8)
project(OpenOMF C)
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake-scripts)

INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)

set(VERSION_MAJOR "0")
set(VERSION_MINOR "6")
set(VERSION_PATCH "5")
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Options
OPTION(USE_LTO "Enable LTO" OFF)
OPTION(USE_TESTS "Build unittests" OFF)
OPTION(USE_OGGVORBIS "Add support for Ogg Vorbis audio" OFF)
OPTION(USE_DUMB "Use libdumb for module playback" ON)
OPTION(USE_MODPLUG "Use libmodplug for module playback" OFF)
OPTION(USE_XMP "Use libxmp for module playback" OFF)
OPTION(USE_PNG "Add support for PNG screenshots" ON)
OPTION(USE_OPENAL "Support OpenAL for audio playback" ON)
OPTION(USE_SUBMODULES "Add libsd and libdumb as submodules" ON)
OPTION(USE_RELEASE_SUBMODULES "Build the submodules in release mode. Enable this option if debug build segfaults on mainmenu." OFF)
#OPTION(SERVER_ONLY "Do not build the game binary" OFF)

# These flags are used for all builds
set(CMAKE_C_FLAGS "-Wall -std=c11")
set(CMAKE_C_FLAGS_DEBUG "-ggdb -DDEBUGMODE -Werror -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2 -fno-omit-frame-pointer -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -DNDEBUG")
add_definitions(-DV_MAJOR=${VERSION_MAJOR} -DV_MINOR=${VERSION_MINOR} -DV_PATCH=${VERSION_PATCH})

# See if we have Git, and use it to fetch current SHA1 hash
find_package(Git)
if(GIT_FOUND)
    message(STATUS "Git found: ${GIT_EXECUTABLE}")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} "rev-parse" "HEAD"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE SHA1_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "Git SHA1 Hash: ${SHA1_HASH}")
    add_definitions(-DSHA1_HASH="${SHA1_HASH}")
endif()

# Enable LTO flags if requested
IF(USE_LTO)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -flto -fno-fat-lto-objects")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
    message(STATUS "Link time optimization enabled!")

    string(TOUPPER ${CMAKE_BUILD_TYPE} BTYPE)
    IF(BTYPE MATCHES DEBUG)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ggdb")
    ELSEIF(BTYPE MATCHES RELEASE)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -O2")
    ELSEIF(BTYPE MATCHES RELWITHDEBINFO)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -O2")
    ELSEIF(BTYPE MATCHES MINSIZEREL)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Os")
    ENDIF()
ENDIF()

# System packages (hard dependencies)
find_package(SDL2)
find_package(enet)
find_package(confuse)

# Check functions and generate platform configuration file
CHECK_SYMBOL_EXISTS(strdup "string.h" HAVE_STD_STRDUP)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/include/platform.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/platform.h)

# Only search for submodules if we want them
IF(USE_SUBMODULES)
    # XXX workaround for Mingw 32 bit 4.6.X - 4.8.X compilers
    # In debug build, the game segfaults inside libdumb's SSE resampling code while loading the mainmenu.
    # Using release build of libdumb works around that problem.
    # If anybody could track down why that segfault occurs I would appreciate it
    if(USE_RELEASE_SUBMODULES)
        set(XXX_OLD_BUILD_TYPE ${CMAKE_BUILD_TYPE})
        set(CMAKE_BUILD_TYPE "Release")
    endif()
    add_subdirectory(external)
    if(USE_RELEASE_SUBMODULES)
        set_target_properties(dumb PROPERTIES DEBUG_POSTFIX "")
        set_target_properties(shadowdive PROPERTIES DEBUG_POSTFIX "")
        set(CMAKE_BUILD_TYPE ${XXX_OLD_BUILD_TYPE})
    endif()

    MESSAGE(STATUS "Using submodule dumb: ${DUMB_INCLUDE_DIR}")
    MESSAGE(STATUS "Using submodule shadowdive: ${SHADOWDIVE_INCLUDE_DIR}")
ELSE()
    find_package(shadowdive)
ENDIF()

# If submodules are in use, or if USE_DUMB flag is on, turn on libdumb
IF(USE_SUBMODULES)
    MESSAGE(STATUS "USE_SUBMODULES=1 flag automatically forces USE_DUMB=1.")
    add_definitions(-DUSE_DUMB)
ELSEIF(USE_DUMB)
    find_package(dumb)
    add_definitions(-DUSE_DUMB)
ENDIF()

# If modplug has been selected, attempt to find it
IF(USE_MODPLUG)
    find_package(modplug)
    add_definitions(-DUSE_MODPLUG)
ENDIF()

# If XMP has been selected, attempt to find it
IF(USE_XMP)
    find_package(xmp)
    add_definitions(-DUSE_XMP)
ENDIF()

# Audio sink selection
IF(USE_OPENAL)
    find_package(OpenAL)
    add_definitions(-DUSE_OPENAL)
ELSE()
    MESSAGE(STATUS "Note! No audio sink selected; Music/sounds will not play.")
ENDIF()

# When building with MingW, do not look for Libintl
# Also, use static libgcc when on mingw
IF(MINGW)
    set(LIBINTL_INCLUDE_DIR "")
    set(LIBINTL_LIBRARIES "")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static-libgcc")
ELSE()
    find_package(Libintl)
ENDIF()

# If we want to build support for vorbis streaming, find these too
IF(USE_OGGVORBIS)
    find_package(ogg)
    find_package(vorbis)
    add_definitions(-DUSE_OGGVORBIS)
ENDIF()

# Find PNG if it is switched on
IF(USE_PNG)
    find_package(PNG)
    find_package(ZLIB)
    add_definitions(-DUSE_PNG)
ENDIF()

# If tests are enabled, find CUnit
IF(USE_TESTS)
    find_package(CUnit)
ENDIF()

# Only strip on GCC (clang does not appreciate)
IF(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wl,-s")
    set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -Wl,-s")
ENDIF()

set(OPENOMF_SRC
    src/utils/compat.c
    src/utils/log.c
    src/utils/config.c
    src/utils/list.c
    src/utils/vector.c
    src/utils/hashmap.c
    src/utils/iterator.c
    src/utils/array.c
    src/utils/vec.c
    src/utils/str.c
    src/utils/random.c
    src/utils/miscmath.c
    src/utils/scandir.c
    src/utils/msgbox.c
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
    src/audio/sources/modplug_source.c
    src/audio/sources/xmp_source.c
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
    src/resources/pilots.c
    src/resources/sprite.c
    src/resources/animation.c
    src/resources/sounds_loader.c
    src/resources/pathmanager.c
    src/resources/sgmanager.c
    src/resources/languages.c
    src/resources/fonts.c
    src/resources/scores.c
    src/plugins/plugins.c
    src/plugins/scaler_plugin.c
    src/game/protos/object.c
    src/game/protos/player.c
    src/game/protos/scene.c
    src/game/protos/intersect.c
    src/game/protos/object_specializer.c
    src/game/objects/har.c
    src/game/objects/scrap.c
    src/game/objects/projectile.c
    src/game/objects/hazard.c
    src/game/scenes/intro.c
    src/game/scenes/openomf.c
    src/game/scenes/mainmenu.c
    src/game/scenes/mainmenu/menu_main.c
    src/game/scenes/mainmenu/menu_configuration.c
    src/game/scenes/mainmenu/menu_gameplay.c
    src/game/scenes/mainmenu/menu_audio.c
    src/game/scenes/mainmenu/menu_video.c
    src/game/scenes/mainmenu/menu_video_confirm.c
    src/game/scenes/mainmenu/menu_net.c
    src/game/scenes/mainmenu/menu_connect.c
    src/game/scenes/mainmenu/menu_listen.c
    src/game/scenes/mainmenu/menu_input.c
    src/game/scenes/mainmenu/menu_keyboard.c
    src/game/scenes/mainmenu/menu_presskey.c
    src/game/scenes/credits.c
    src/game/scenes/cutscene.c
    src/game/scenes/arena.c
    src/game/scenes/melee.c
    src/game/scenes/vs.c
    src/game/scenes/mechlab.c
    src/game/scenes/mechlab/lab_menu_main.c
    src/game/scenes/mechlab/lab_menu_customize.c
    src/game/scenes/mechlab/lab_menu_training.c
    src/game/scenes/mechlab/lab_menu_pilotselect.c
    src/game/scenes/mechlab/lab_dash_main.c
    src/game/scenes/mechlab/lab_dash_newplayer.c
    src/game/scenes/newsroom.c
    src/game/scenes/scoreboard.c
    src/game/objects/har.c
    src/game/gui/menu.c
    src/game/gui/trn_menu.c
    src/game/gui/menu_background.c
    src/game/gui/text_render.c
    src/game/gui/textbutton.c
    src/game/gui/spritebutton.c
    src/game/gui/spriteimage.c
    src/game/gui/textselector.c
    src/game/gui/textslider.c
    src/game/gui/textinput.c
    src/game/gui/dialog.c
    src/game/gui/component.c
    src/game/gui/sizer.c
    src/game/gui/widget.c
    src/game/gui/frame.c
    src/game/gui/label.c
    src/game/gui/filler.c
    src/game/gui/progressbar.c
    src/game/gui/gauge.c
    src/game/gui/pilotpic.c
    src/game/gui/xysizer.c
    src/game/game_state.c
    src/game/game_player.c
    src/game/common_defines.c
    src/game/utils/ticktimer.c
    src/game/utils/serial.c
    src/game/utils/settings.c
    src/game/utils/score.c
    src/game/utils/har_screencap.c
    src/game/utils/formatting.c
    src/controller/controller.c
    src/controller/keyboard.c
    src/controller/joystick.c
    src/controller/net_controller.c
    src/controller/ai_controller.c
    src/controller/rec_controller.c
    src/console/console.c
    src/console/console_cmd.c
    src/engine.c
)

set(COREINCS
    include
    ${CMAKE_CURRENT_BINARY_DIR}/include/
    ${SDL2_INCLUDE_DIR}
    ${CONFUSE_INCLUDE_DIR}
    ${SHADOWDIVE_INCLUDE_DIR}
    ${LIBINTL_INCLUDE_DIR}
    ${ENET_INCLUDE_DIR}
)

set(CORELIBS
    ${SDL2_LIBRARY}
    ${CONFUSE_LIBRARY}
    ${LIBINTL_LIBRARIES}
    ${ENET_LIBRARY}
)

# Shadowdive
IF(USE_SUBMODULES)
    set(CORELIBS ${CORELIBS} shadowdive)
ELSE()
    set(CORELIBS ${CORELIBS} ${SHADOWDIVE_LIBRARY})
ENDIF()

# Handle module playback libraries
IF(USE_SUBMODULES)
    set(CORELIBS ${CORELIBS} dumb)
    set(COREINCS ${COREINCS} ${DUMB_INCLUDE_DIR})
ELSEIF(USE_DUMB)
    set(CORELIBS ${CORELIBS} ${DUMB_LIBRARY})
    set(COREINCS ${COREINCS} ${DUMB_INCLUDE_DIR})
ENDIF()
IF(USE_MODPLUG)
    set(CORELIBS ${CORELIBS} ${MODPLUG_LIBRARY})
    set(COREINCS ${COREINCS} ${MODPLUG_INCLUDE_DIR})
ENDIF()
IF(USE_XMP)
    set(CORELIBS ${CORELIBS} ${XMP_LIBRARY})
    set(COREINCS ${COREINCS} ${XMP_INCLUDE_DIR})
ENDIF()

# Audio sinks
IF(USE_OPENAL)
    set(CORELIBS ${CORELIBS} ${OPENAL_LIBRARY})
    set(COREINCS ${COREINCS} ${OPENAL_INCLUDE_DIR})
ENDIF()

# If we support ogg vorbis, add library includes etc.
IF(USE_OGGVORBIS)
    set(COREINCS ${COREINCS} ${OGG_INCLUDE_DIR} ${VORBIS_INCLUDE_DIR})
    set(CORELIBS ${CORELIBS} ${VORBISFILE_LIBRARY} ${VORBIS_LIBRARY} ${OGG_LIBRARY})
ENDIF()

# If we support PNG, add library incs etc.
IF(USE_PNG)
    set(COREINCS ${COREINCS} ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
    set(CORELIBS ${CORELIBS} ${PNG_LIBRARY} ${ZLIB_LIBRARY})
ENDIF()

# MingW build should add mingw32 lib
IF(MINGW)
    set(CORELIBS mingw32 ${CORELIBS})
ENDIF()

# On windows, add winsock2 and winmm
IF(WIN32)
    set(CORELIBS ${CORELIBS} ws2_32 winmm)
ENDIF()

# On unix platforms, add libm (sometimes needed, it seems)
IF(UNIX)
    SET(CORELIBS ${CORELIBS} -lm)
ENDIF()

include_directories(${COREINCS})

# Build the server binary
# XXX uncomment this later when we start working on the dedicated server
#add_executable(openomf_server ${OPENOMF_SRC} src/main.c)
#set_target_properties(openomf_server PROPERTIES COMPILE_DEFINITIONS "STANDALONE_SERVER=1")
#target_link_libraries(openomf_server ${CORELIBS})

# Build the game binary
IF(NOT SERVER_ONLY)
    add_executable(openomf ${OPENOMF_SRC} src/main.c)
    # Don't show console on mingw in release builds
    IF(MINGW)
        IF(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
            set_target_properties(openomf PROPERTIES LINK_FLAGS "-mwindows")
        ENDIF(NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
    ENDIF(MINGW)
    target_link_libraries(openomf ${CORELIBS})
ENDIF(NOT SERVER_ONLY)

# Testing stuff
IF(CUNIT_FOUND)
    include_directories(${CUNIT_INCLUDE_DIR} testing/ include/)
    SET(CORELIBS ${CORELIBS} ${CUNIT_LIBRARY})

    add_executable(openomf_test_main
        testing/test_main.c
        testing/test_str.c
        testing/test_hashmap.c
        testing/test_vector.c
        testing/test_list.c
        testing/test_array.c
        testing/test_text_render.c
        ${OPENOMF_SRC}
    )

    target_link_libraries(openomf_test_main ${CORELIBS})

    # Change policy to allow "test" target name
    cmake_policy(PUSH)
    if(POLICY CMP0037)
        cmake_policy(SET CMP0037 OLD)
    endif()
    add_custom_target(test openomf_test_main)
    cmake_policy(POP)
ENDIF(CUNIT_FOUND)

# Packaging
add_subdirectory(packaging)

# Installation
INSTALL(TARGETS openomf
    RUNTIME
    DESTINATION bin
    COMPONENT Binaries
)
INSTALL(FILES resources/openomf.bk resources/openomf_icon.png
    DESTINATION share/openomf/
    COMPONENT Data
)
