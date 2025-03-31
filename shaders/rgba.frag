#version 330 core

// In
in vec2 tex_coord;
layout (std140) uniform palette {
    vec4 colors[256];
};

uniform sampler2D framebuffer;
uniform sampler2D remaps;
uniform uint framebuffer_options;

// for info about these options, see renderer_framebuffer_options in video/enums.h
bool FBUFOPT_CREDITS = (framebuffer_options & 0x01u) != 0u;


// Out
layout (location = 0) out vec4 color;

void main() {
    vec4 texel = texture(framebuffer, tex_coord);

    if(FBUFOPT_CREDITS) {
        // SPRITE_INDEX_ADD
        int index = int((texel.r + texel.a) * 255.0);
        color = colors[index];
        return;
    }

    int remap = int(texel.g * 255.0);
    float remap_index = float(remap % 19) / 18.0;
    int remap_rounds = remap / 19;

    // SPRITE_INDEX_ADD
    texel.r += texel.a;

    // SPRITE_DARK_TINT
    int sprite_darktint = int(texel.b * 255.0);
    int magic_remap_rounds = 12;
    if(sprite_darktint > 0 && remap_rounds != magic_remap_rounds){
        // DARK_TINT's remap got trampled by the pause menu,
        // so the HAR's color with the pause menu remap.
        texel.r = texel.b;
    } else if(sprite_darktint >= 0x60) {
        // pyros flames draw opaque, no remaps.
        texel.r = texel.b;
        remap_rounds = 0;
    } else if(sprite_darktint > 0) {
        // lookup color we're drawing ontop in fifth remap to get brightness
        vec4 remap = texture(remaps, vec2(texel.r, 4.0 / 18.0));
        int behind = 1 + clamp(int(remap.r * 255.0) - 0xA8, 0, 7) * 2;

        // this indexed blending is not exactly correct, but i am
        // done fiddling with it for now-- if someone finds the
        // code for this in MASTER.DAT, then we can correct it.
        sprite_darktint = (sprite_darktint & 0xF0) + ((sprite_darktint & 0x0F) * 3 + behind * 2) / 5;
        texel.r = float(sprite_darktint) / 255.0;
    }

    // TODO: precalculate palette to 2d texture for required remappings later.
    for (int i = 0; i < remap_rounds; i++) {
        texel = texture(remaps, vec2(texel.r, remap_index));
    }

    color = colors[int(255.0 * texel.r)];
}