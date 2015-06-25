#include <stdio.h>
#include <signal.h> // signal()
#include <SDL2/SDL.h>
#include "engine.h"
#include "utils/log.h"
#include "utils/config.h"
#include "audio/audio.h"
#include "audio/music.h"
#include "resources/sounds_loader.h"
#include "video/surface.h"
#include "video/video.h"
#include "resources/languages.h"
#include "game/game_state.h"
#include "game/utils/settings.h"
#include "game/utils/ticktimer.h"
#include "game/gui/text_render.h"
#include "console/console.h"

static int run = 0;
static int start_timeout = 30;
#ifndef STANDALONE_SERVER
static int take_screenshot = 0;
static int enable_screen_updates = 1;
static char screenshot_filename[128];
#endif

void exit_handler(int s) {
    run = 0;
}

int engine_init() {
#ifndef STANDALONE_SERVER
    settings *setting = settings_get();

    int w = setting->video.screen_w;
    int h = setting->video.screen_h;
    int fs = setting->video.fullscreen;
    int vsync = setting->video.vsync;
    int scale_factor = setting->video.scale_factor;
    char *scaler = setting->video.scaler;
    const char *audiosink = setting->sound.sink;

    // Initialize everything.
    if(video_init(w, h, fs, vsync, scaler, scale_factor)) {
        goto exit_0;
    }
    if(!audio_is_sink_available(audiosink)) {
        const char *prev_sink = audiosink;
        audiosink = audio_get_first_sink_name();
        if(audiosink == NULL) {
            INFO("Could not find requested sink '%s'. No other sink available; disabling audio.", prev_sink);
        } else {
            INFO("Could not find requested sink '%s'. Falling back to '%s'.", prev_sink, audiosink);
        }
    }
    if(audio_init(audiosink)) {
        goto exit_1;
    }
    sound_set_volume(setting->sound.sound_vol/10.0f);
    music_set_volume(setting->sound.music_vol/10.0f);
#endif

    if(sounds_loader_init()) {
        goto exit_2;
    }
    if(lang_init()) {
        goto exit_3;
    }
    if(fonts_init()) {
        goto exit_4;
    }
    if(altpals_init()) {
        goto exit_5;
    }
    if(console_init()) {
        goto exit_6;
    }

    // Return successfully
    run = 1;
    INFO("Engine initialization successful.");
    return 0;

    // If something failed, close in correct order
exit_6:
    altpals_close();
exit_5:
    fonts_close();
exit_4:
    lang_close();
exit_3:
    sounds_loader_close();

exit_2:
#ifndef STANDALONE_SERVER
    audio_close();
#endif

exit_1:
#ifndef STANDALONE_SERVER
    video_close();
#endif

exit_0:
    return 1;
}

void engine_run(engine_init_flags *init_flags) {
    SDL_Event e;
    int visual_debugger = 0;
    int debugger_proceed = 0;
    int debugger_render = 0;

    //if mouse_visible_ticks <= 0, hide mouse
    int mouse_visible_ticks = 1000;

    INFO(" --- BEGIN GAME LOG ---");

#ifdef STANDALONE_SERVER
    // Init interrupt signal handler
    signal(SIGINT, exit_handler);
#endif

#ifndef STANDALONE_SERVER
    // Game start timeout.
    // Wait a moment so that people are mentally prepared
    // (with the recording software on) for the game to start :)
    if(!settings_get()->video.crossfade_on) {
        start_timeout = 0;
    }
    while(start_timeout > 0) {
        start_timeout--;
        while(SDL_PollEvent(&e)) {
            if(e.type == SDL_QUIT) {
                return;
            }
        }
        video_render_prepare();
        video_render_finish();
        continue;
    }

    // apply volume settings
    sound_set_volume(settings_get()->sound.sound_vol/10.0f);
#endif

    // Set up game
    game_state *gs = malloc(sizeof(game_state));
    if(game_state_create(gs, init_flags)) {
        return;
    }

    // Game loop
    int frame_start = SDL_GetTicks();
    int dynamic_wait = 0;
    int static_wait = 0;
    while(run && game_state_is_running(gs)) {

#ifndef STANDALONE_SERVER
        // Handle events
        int check_fs;
        while(SDL_PollEvent(&e)) {
            // Handle other events
            switch(e.type) {
                case SDL_QUIT:
                    run = 0;
                    break;
                case SDL_KEYDOWN:
                    if(e.key.keysym.sym == SDLK_F1) {
                        take_screenshot = 1;
                    }
                    if(e.key.keysym.sym == SDLK_F5) {
                        visual_debugger = !visual_debugger;
                    }
                    if(e.key.keysym.sym == SDLK_SPACE) {
                        debugger_proceed = 1;
                    }
                    if(e.key.keysym.sym == SDLK_F6) {
                        debugger_render = !debugger_render;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    mouse_visible_ticks = 1000;
                    SDL_ShowCursor(1);
                    break;
                case SDL_WINDOWEVENT:
                    switch(e.window.event) {
                        case SDL_WINDOWEVENT_MINIMIZED:
                            DEBUG("MINIMIZED");
                            enable_screen_updates = 0;
                            break;
                        case SDL_WINDOWEVENT_HIDDEN:
                            DEBUG("HIDDEN");
                            enable_screen_updates = 0;
                            break;
                        case SDL_WINDOWEVENT_MAXIMIZED:
                            DEBUG("MAXIMIZED");
                            enable_screen_updates = 1;
                            break;
                        case SDL_WINDOWEVENT_RESTORED:
                            video_get_state(NULL, NULL, &check_fs, NULL);
                            if(check_fs) {
                                video_reinit_renderer();
                            }
                            DEBUG("RESTORED");
                            enable_screen_updates = 1;
                            break;
                        case SDL_WINDOWEVENT_SHOWN:
                            enable_screen_updates = 1;
                            DEBUG("SHOWN");
                            break;
                    }
                    break;
            }

            // Console events
            if(e.type == SDL_KEYDOWN) {
                if(console_window_is_open() && (e.key.keysym.scancode == SDL_SCANCODE_GRAVE ||
                                                e.key.keysym.sym == SDLK_BACKQUOTE ||
                                                e.key.keysym.sym == SDLK_TAB ||
                                                e.key.keysym.sym == SDLK_ESCAPE)) {
                    console_window_close();
                    continue;
                } else if(e.key.keysym.sym == SDLK_TAB ||
                          e.key.keysym.sym == SDLK_BACKQUOTE ||
                          e.key.keysym.scancode == SDL_SCANCODE_GRAVE) {
                    console_window_open();
                    continue;
                }
            }

            // If console windows is open, pass events to console.
            // Otherwise to the objects.
            if(console_window_is_open()) {
                console_event(gs, &e);
            } else {
                game_state_handle_event(gs, &e);
            }
        }

        // hide mouse after n ticks
        if(mouse_visible_ticks > 0) {
            mouse_visible_ticks -= SDL_GetTicks() - frame_start;
            if(mouse_visible_ticks <= 0) {
                SDL_ShowCursor(0);
            }
        }
#endif
        // Tick controllers
        game_state_tick_controllers(gs);

        // Render scene
        int dt = (SDL_GetTicks() - frame_start);
        frame_start = SDL_GetTicks(); // Reset timer
        if(!visual_debugger) {
            dynamic_wait += dt;
            static_wait += dt;
        } else if(debugger_proceed) {
            dynamic_wait += 20;
            static_wait += 20;
            debugger_proceed = 0;
        }
        while(static_wait > 10) {
            // Static tick for gamestate
            game_state_static_tick(gs);

            // Tick console
            console_tick();

            // Tick video (tcache)
            video_tick();

            static_wait -= 10;
        }
        while(dynamic_wait > game_state_ms_per_dyntick(gs)) {
            // Tick scene
            game_state_dynamic_tick(gs);

            // Handle waiting period leftover time
            dynamic_wait -= game_state_ms_per_dyntick(gs);
        }

#ifndef STANDALONE_SERVER
        // Handle audio
        if(!visual_debugger) {
            audio_render();
        }

        // Do the actual video rendering jobs
        if(enable_screen_updates) {

            video_render_prepare();
            game_state_render(gs);
            if(debugger_render) {
                game_state_debug(gs);
            }
            console_render();
            video_render_finish();

            // If screenshot requested, do it here.
            if(take_screenshot) {
                image img;
                video_screenshot(&img);
                int scr_ret = 0;
                if(image_supports_png()) {
                    sprintf(screenshot_filename, "screenshot_%u.png", SDL_GetTicks());
                    scr_ret = image_write_png(&img, screenshot_filename);
                } else {
                    sprintf(screenshot_filename, "screenshot_%u.tga", SDL_GetTicks());
                    scr_ret= image_write_tga(&img, screenshot_filename);
                }
                if(scr_ret) {
                    PERROR("Screenshot write operation failed (%s)", screenshot_filename);
                } else {
                    DEBUG("Got a screenshot: %s", screenshot_filename);
                }

                image_free(&img);
                take_screenshot = 0;
            }
        } else {
            // If screen updates are disabled, then wait
            SDL_Delay(1);
        }
#else
        // In standalone, just wait.
        SDL_Delay(1);
#endif // STANDALONE_SERVER
    }

    // Free scene object
    game_state_free(gs);
    free(gs);

    INFO(" --- END GAME LOG ---");
}

void engine_close() {
    console_close();
    altpals_close();
    fonts_close();
    lang_close();
    sounds_loader_close();
#ifndef STANDALONE_SERVER
    audio_close();
    video_close();
#endif
    INFO("Engine deinit successful.");
}
