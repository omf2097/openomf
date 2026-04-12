#version 330 core

// In
in vec2 tex_coord;
uniform sampler2D palette;

// Paletted framebuffer (GL_RGBA16), written by palette.frag.
// Channels are built up by multiple draw passes with different blend modes:
//   R: palette index / 1023.0 (MODE_SET)
//   G: (remap_offset + remap_rounds * 19 [+ index]) / 255.0 (MODE_REMAP / MODE_SPRITE_SHADOW / MODE_DARK_TINT)
//   B: dark tint palette index / 1023.0  (MODE_DARK_TINT)
//   A: additive index (index * 60) / 1023.0 (MODE_ADD)
uniform sampler2D framebuffer;

// Remap texture (GL_R16UI, 1024x19): uint16 palette index (0-1023) per texel.
// X axis is the source palette index, Y axis selects one of 19 remap tables.
uniform usampler2D remaps;

uniform uint framebuffer_options;

// for info about these options, see renderer_framebuffer_options in video/enums.h
bool FBUFOPT_CREDITS = (framebuffer_options & 0x01u) != 0u;

// It's magic!
const int MAGIC_REMAP_ROUNDS = 12;

// Out
layout (location = 0) out vec4 color;

void set_color(int index) {
    ivec2 pal_index = ivec2(index, 0);
    color = texelFetch(palette, pal_index, 0);
}

void main() {
    vec4 texel = textureLod(framebuffer, tex_coord, 0);
    int idx = int(texel.r * 1023.0 + 0.5);
    int remap_enc = int(texel.g * 255.0);
    int darktint = int(texel.b * 1023.0 + 0.5);
    int idx_add = int(texel.a * 1023.0 + 0.5);

    if(FBUFOPT_CREDITS) {
        // SPRITE_INDEX_ADD
        set_color(idx + idx_add);
        return;
    }

    int remap_row = remap_enc % 19;
    int remap_rounds = remap_enc / 19;

    // SPRITE_INDEX_ADD
    idx += idx_add;

    // SPRITE_DARK_TINT
    if(darktint > 0 && remap_rounds != MAGIC_REMAP_ROUNDS){
        // DARK_TINT's remap got trampled by the pause menu,
        // so the HAR's color with the pause menu remap.
        idx = darktint;
    } else if(darktint >= 0x60) {
        // pyros flames draw opaque, no remaps.
        idx = darktint;
        remap_rounds = 0;
    } else if(darktint > 0) {
        // lookup color we're drawing ontop in fifth remap to get brightness
        uvec4 remap_val = texelFetch(remaps, ivec2(idx, 4), 0);
        int behind = 1 + clamp(int(remap_val.r) - 0xA8, 0, 7) * 2;

        // this indexed blending is not exactly correct, but i am
        // done fiddling with it for now-- if someone finds the
        // code for this in MASTER.DAT, then we can correct it.
        idx = (darktint & 0xF0) + ((darktint & 0x0F) * 3 + behind * 2) / 5;
    }

    // TODO: precalculate palette to 2d texture for required remappings later.
    for (int i = 0; i < remap_rounds; i++) {
        uvec4 remap_texel = texelFetch(remaps, ivec2(idx, remap_row), 0);
        idx = int(remap_texel.r);
    }

    set_color(idx);
}
