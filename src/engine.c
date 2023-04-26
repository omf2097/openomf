#include "engine.h"
#include "audio/audio.h"
#include "console/console.h"
#include "controller/controller.h"
#include "formats/altpal.h"
#include "game/game_player.h"
#include "game/game_state.h"
#include "game/gui/text_render.h"
#include "game/utils/settings.h"
#include "resources/languages.h"
#include "resources/sounds_loader.h"
#include "utils/allocator.h"
#include "utils/log.h"
#include "video/surface.h"
#include "video/video.h"
#include <SDL.h>
#include <stdio.h>

static int run = 0;
static int start_timeout = 30;
static int take_screenshot = 0;
static int enable_screen_updates = 1;
static char screenshot_filename[128];

int engine_init(void) {
    settings *setting = settings_get();

    int w = setting->video.screen_w;
    int h = setting->video.screen_h;
    int fs = setting->video.fullscreen;
    int vsync = setting->video.vsync;
    int frequency = setting->sound.music_frequency;
    int resampler = setting->sound.music_resampler;
    bool mono = setting->sound.music_mono;
    float music_volume = setting->sound.music_vol / 10.0;
    float sound_volume = setting->sound.sound_vol / 10.0;

    // Initialize everything.
    if(video_init(w, h, fs, vsync))
        goto exit_0;
    if(!audio_init(frequency, mono, resampler, music_volume, sound_volume))
        goto exit_1;
    if(sounds_loader_init())
        goto exit_2;
    if(lang_init())
        goto exit_3;
    if(fonts_init())
        goto exit_4;
    if(altpals_init())
        goto exit_5;
    if(console_init())
        goto exit_6;

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
    audio_close();
exit_1:
    video_close();
exit_0:
    return 1;
}

void engine_run(engine_init_flags *init_flags) {
    SDL_Event e;
    int visual_debugger = 0;
    int debugger_proceed = 0;
    int debugger_render = 0;

    // if mouse_visible_ticks <= 0, hide mouse
    uint64_t mouse_visible_ticks = 1000;

    INFO(" --- BEGIN GAME LOG ---");

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
    }

    // apply volume settings
    audio_set_sound_volume(settings_get()->sound.sound_vol / 10.0f);

    // Set up game
    game_state *gs = omf_calloc(1, sizeof(game_state));
    if(game_state_create(gs, init_flags)) {
        game_state_free(&gs);
        return;
    }

    // Game loop
    uint64_t frame_start = SDL_GetTicks64(); // Set game tick timer
    int dynamic_wait = 0;
    int static_wait = 0;
    while(run && game_state_is_running(gs)) {
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
                if(console_window_is_open() &&
                   (e.key.keysym.scancode == SDL_SCANCODE_GRAVE || e.key.keysym.sym == SDLK_BACKQUOTE ||
                    e.key.keysym.sym == SDLK_TAB || e.key.keysym.sym == SDLK_ESCAPE)) {
                    console_window_close();
                    continue;
                } else if(e.key.keysym.sym == SDLK_TAB || e.key.keysym.sym == SDLK_BACKQUOTE ||
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
            mouse_visible_ticks -= SDL_GetTicks64() - frame_start;
            if(mouse_visible_ticks <= 0) {
                SDL_ShowCursor(0);
            }
        }

        // Tick controllers
        game_state_tick_controllers(gs);

        // check if we need to replace the game state
        if(gs->new_state) {
            // one of the controllers wants to replace the game state
            game_state *old_gs = gs;
            gs = gs->new_state;
            DEBUG("replacing game state! %d %d", old_gs, gs);
            // old_gs->new_state = NULL;
            game_state_clone_free(old_gs);
            omf_free(old_gs);
        }

        // Render scene
        uint64_t frame_dt = SDL_GetTicks64() - frame_start;
        frame_start = SDL_GetTicks64();
        if(!visual_debugger) {
            dynamic_wait += frame_dt;
            static_wait += frame_dt;
        } else if(debugger_proceed) {
            dynamic_wait += 20;
            static_wait += 20;
            debugger_proceed = 0;
        }
        int limit_static = 100;
        int limit_dynamic = 100;
        while(static_wait > 10 && limit_static--) {
            // Static tick for game state
            game_state_static_tick(gs, false);

            // Tick console
            console_tick();

            // Tick video (tcache)
            video_tick();

            static_wait -= 10;
        }
        while(dynamic_wait > game_state_ms_per_dyntick(gs) && limit_dynamic--) {
            // Tick scene
            game_state_dynamic_tick(gs, false);

            // Handle waiting period leftover time
            dynamic_wait -= game_state_ms_per_dyntick(gs);
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
                int failed_screenshot = video_screenshot(&img);
                if(!failed_screenshot) {
                    snprintf(screenshot_filename, 128, "screenshot_%llu.png", SDL_GetTicks64());
                    int scr_ret = image_write_png(&img, screenshot_filename);
                    if(scr_ret) {
                        PERROR("Screenshot write operation failed (%s)", screenshot_filename);
                    } else {
                        DEBUG("Got a screenshot: %s", screenshot_filename);
                    }
                }

                image_free(&img);
                take_screenshot = 0;
            }
        } else {
            // If screen updates are disabled, then wait
            SDL_Delay(1);
        }
    }

    // Free scene object
    game_state_free(&gs);

    INFO(" --- END GAME LOG ---");
}

void engine_close(void) {
    console_close();
    altpals_close();
    fonts_close();
    lang_close();
    sounds_loader_close();
    audio_close();
    video_close();
    INFO("Engine deinit successful.");
}
