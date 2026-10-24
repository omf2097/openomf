#version 330 core

// In
in vec2 tex_coord;
uniform sampler2D palette;

// Paletted framebuffer (GL_RGBA16), written by palette.frag.
// Channels are built up by multiple draw passes with different blend modes:
//   R: palette index / 1023.0 (MODE_SET)
//   G: (remap_offset + remap_rounds * 19 [+ index]) / 255.0 (MODE_REMAP / MODE_SPRITE_SHADOW)
//   B: translucent sprite palette index + 1, 0 = none (MODE_TRANSLUCENT)
//   A: additive index (index * 60) / 1023.0 (MODE_ADD)
uniform sampler2D framebuffer;

// Remap texture (GL_R16UI, 1024x19): uint16 palette index (0-1023) per texel.
// X axis is the source palette index, Y axis selects one of 19 remap tables.
uniform usampler2D remaps;

uniform uint framebuffer_options;

// for info about these options, see renderer_framebuffer_options in video/enums.h
bool FBUFOPT_CREDITS = (framebuffer_options & 0x01u) != 0u;

// Out
layout (location = 0) out vec4 color;

void set_color(int index) {
    ivec2 pal_index = ivec2(index, 0);
    color = texelFetch(palette, pal_index, 0);
}

int brightness(int index) {
    ivec3 rgb = ivec3(texelFetch(palette, ivec2(index, 0), 0).rgb * 255.0 + 0.5);
    return (rgb.r + rgb.g + rgb.b) / 3;
}

void main() {
    vec4 texel = textureLod(framebuffer, tex_coord, 0);
    int idx = int(texel.r * 1023.0 + 0.5);
    int remap_enc = int(texel.g * 255.0);
    int translucent = int(texel.b * 1023.0 + 0.5);
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

    // SPRITE_TRANSLUCENT
    if (translucent > 0) {
        translucent -= 1;
        if (translucent < 0x80) {
            idx = translucent + (brightness(idx) >> 5);
        } else {
            idx = translucent;
        }
    }

    // TODO: precalculate palette to 2d texture for required remappings later.
    for (int i = 0; i < remap_rounds; i++) {
        uvec4 remap_texel = texelFetch(remaps, ivec2(idx, remap_row), 0);
        idx = int(remap_texel.r);
    }

    set_color(idx);
}
