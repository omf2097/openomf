#ifndef _SETTINGS_H
#define _SETTINGS_H

typedef enum fight_mode_t {
    FIGHT_MODE_NORMAL, 
    FIGHT_MODE_HYPER
} fight_mode;

typedef enum difficulty_t {
    PUNCHING_BAG, 
    ROOKIE, 
    VETERAN, 
    WORLD_CLASS,
    CHAMPION,
    DEADLY,
    ULTIMATE
} difficulty;

typedef struct settings_sound_t {
    int sound_on;
    int music_on;
    int stereo_on;
    int stereo_reversed;
    int sound_vol;
    int music_vol;
} settings_sound;

typedef struct settings_video_t {
    int resindex;
    int screen_w;
    int screen_h;
    int vsync;
    int fullscreen;
    int scaling;
} settings_video;

typedef struct settings_gameplay_t {
    int speed;
    int fight_mode;
    int power1;
    int power2;
    int hazards_on;
    int difficulty;
    int rounds;
} settings_gameplay;

typedef struct settings_t {
    settings_video video;
    settings_sound sound;
    settings_gameplay gameplay;
} settings;

int settings_init(settings *s);
void settings_free(settings *s);

void settings_load(settings *s);
void settings_save(settings *s);

#endif // _SETTINGS_H

