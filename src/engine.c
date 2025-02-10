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
#include "utils/miscmath.h"
#include "utils/png_writer.h"
#include "utils/time_fmt.h"
#include "video/vga_state.h"
#include "video/video.h"
#include <SDL.h>
#include <stdio.h>

#define MAX_TICKS_PER_FRAME 10
#define TICK_EXPIRY_MS 100

static int run = 0;
static int start_timeout = 30;
static int enable_screen_updates = 1;
static int debug_palette_number = 0;

int engine_init(engine_init_flags *init_flags) {
    settings *setting = settings_get();

    int w = setting->video.screen_w;
    int h = setting->video.screen_h;
    int fs = setting->video.fullscreen;
    int vsync = setting->video.vsync;
    int frequency = setting->sound.sample_rate;
    int resampler = setting->sound.music_resampler;
    bool mono = setting->sound.music_mono;
    float music_volume = setting->sound.music_vol / 10.0;
    float sound_volume = setting->sound.sound_vol / 10.0;
    const char *player = setting->sound.player;
    const char *renderer = setting->video.renderer;
    if(strlen(init_flags->force_audio_backend) > 0)
        player = init_flags->force_audio_backend;
    if(strlen(init_flags->force_renderer) > 0)
        renderer = init_flags->force_renderer;

    // Initialize everything.
    video_scan_renderers();
    audio_scan_backends();
    if(!video_init(renderer, w, h, fs, vsync))
        goto exit_0;
    if(!audio_init(player, frequency, mono, resampler, music_volume, sound_volume))
        goto exit_1;
    if(!sounds_loader_init())
        goto exit_2;
    if(!lang_init())
        goto exit_3;
    if(!fonts_init())
        goto exit_4;
    if(altpals_init())
        goto exit_5;
    if(!console_init())
        goto exit_6;
    vga_state_init();

    // Return successfully
    run = 1;
    log_info("Engine initialization successful.");
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

void save_screenshot(const SDL_Rect *r, unsigned char *data, bool flip) {
    char *time = format_time();
    char *filename = omf_malloc(256);
    snprintf(filename, 256, "screenshot_%s.png", time);
    if(write_rgb_png(filename, r->w, r->h, data, false, flip)) {
        log_debug("Got a screenshot: %s", filename);
    } else {
        log_error("Screenshot write operation failed (%s)", filename);
    }
    omf_free(filename);
    omf_free(time);
}

void save_palette_shot(void) {
    char *time = format_time();
    char *filename = omf_malloc(256);
    snprintf(filename, 256, "debug_palette_%s_%d.png", time, debug_palette_number++);
    vga_state_debug_screenshot(filename);
    log_debug("Palette saved: %s", filename);
    omf_free(filename);
    omf_free(time);
}

void engine_run(engine_init_flags *init_flags) {
    SDL_Event e;
    int visual_debugger = 0;
    int debugger_proceed = 0;
    int debugger_render = 0;

    // if mouse_visible_ticks <= 0, hide mouse
    uint64_t mouse_visible_ticks = 1000;

    log_info(" --- BEGIN GAME LOG ---");

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

    joystick_init();

    // Game loop
    uint64_t frame_start = SDL_GetTicks64(); // Set game tick timer
    int dynamic_wait = 0;
    int static_wait = 0;
    while(run && game_state_is_running(gs)) {
        // Handle events
        bool check_fs;
        while(SDL_PollEvent(&e)) {
            // Handle other events
            switch(e.type) {
                case SDL_QUIT:
                    run = 0;
                    break;
                case SDL_KEYDOWN:
                    if(e.key.keysym.sym == SDLK_F1) {
                        video_schedule_screenshot(save_screenshot);
                    }
                    if(e.key.keysym.sym == SDLK_F2) {
                        save_palette_shot();
                    }
                    if(e.key.keysym.sym == SDLK_F9) {
                        video_draw_atlas(true);
                    }
                    if(e.key.keysym.sym == SDLK_F10) {
                        video_draw_atlas(false);
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
                case SDL_JOYDEVICEADDED:
                    joystick_deviceadded(e.jdevice.which);
                    break;
                case SDL_JOYDEVICEREMOVED:
                    joystick_deviceremoved(e.jdevice.which);
                    break;
                case SDL_MOUSEMOTION:
                    mouse_visible_ticks = 1000;
                    SDL_ShowCursor(1);
                    break;
                case SDL_WINDOWEVENT:
                    switch(e.window.event) {
                        case SDL_WINDOWEVENT_MINIMIZED:
                            log_debug("MINIMIZED");
                            enable_screen_updates = 0;
                            break;
                        case SDL_WINDOWEVENT_HIDDEN:
                            log_debug("HIDDEN");
                            enable_screen_updates = 0;
                            break;
                        case SDL_WINDOWEVENT_MAXIMIZED:
                            log_debug("MAXIMIZED");
                            enable_screen_updates = 1;
                            break;
                        case SDL_WINDOWEVENT_RESTORED:
                            video_get_state(NULL, NULL, &check_fs, NULL);
                            if(check_fs) {
                                video_reinit_renderer();
                            }
                            log_debug("RESTORED");
                            enable_screen_updates = 1;
                            break;
                        case SDL_WINDOWEVENT_SHOWN:
                            enable_screen_updates = 1;
                            log_debug("SHOWN");
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

        // drop ticks if it's been too long since they were due
        dynamic_wait = min2(dynamic_wait, TICK_EXPIRY_MS);
        static_wait = min2(static_wait, TICK_EXPIRY_MS);

        // In warp mode, allow more ticks to happen per vsync period.
        bool has_dynamic = true;
        bool has_static = true;
        int tick_limit = MAX_TICKS_PER_FRAME;
        do {
            // Tick static features. This is a fixed with-rate tick, and is meant for running things
            // that are not dependent on game speed (such as menus).
            has_static = static_wait > STATIC_TICKS;
            if(has_static) {
                game_state_static_tick(gs, false);
                // check if we need to replace the game state
                if(gs->new_state) {
                    // one of the controllers wants to replace the game state
                    game_state *old_gs = gs;
                    game_state *new_gs = gs->new_state;
                    log_debug("replacing game state! %p %p", old_gs, new_gs);
                    gs = new_gs;
                    log_debug("gs new state %p", gs->new_state);
                    // gs->new_state = NULL;
                    game_state_clone_free(old_gs);
                    omf_free(old_gs);
                }
                console_tick(gs);
                static_wait -= STATIC_TICKS;
            }

            // Tick dynamic features. This is a dynamically changing tick, and it depends on things such as
            // hit-pause, hit slowdown and game-speed slider. It is meant for ticking everything that has to do
            // with the actual gameplay stuff.
            has_dynamic = dynamic_wait > game_state_ms_per_dyntick(gs);
            if(has_dynamic) {
                game_state_dynamic_tick(gs, false);
                dynamic_wait -= game_state_ms_per_dyntick(gs);
            }

            // Ensure any pending palette changes are handled after any ticks are made.
            if(has_dynamic || has_static) {
                game_state_palette_transform(gs);
                vga_state_render();
            }
        } while(tick_limit-- && (has_dynamic || has_static));

        // Do the actual video rendering jobs
        if(enable_screen_updates) {
            video_render_prepare();
            game_state_render(gs);
            if(debugger_render) {
                game_state_debug(gs);
            }
            console_render();
            video_render_finish();
        } else {
            // If screen updates are disabled, then wait
            SDL_Delay(1);
        }
    }

    joystick_close();

    // Free scene object
    game_state_free(&gs);

    log_info(" --- END GAME LOG ---");
}

void engine_close(void) {
    console_close();
    altpals_close();
    fonts_close();
    lang_close();
    sounds_loader_close();
    audio_close();
    video_close();
    vga_state_close();
    log_info("Engine deinit successful.");
}
