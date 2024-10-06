#version 330 core

// Palette color band min values
// These are only somewhat correct ...
const int lows[32] = int[32](
0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
0x60, 0x60, 0x60, 0x60,
0x80, 0x80, 0x80, 0x80,
0xA0, 0xA0,
0xB0, 0xB0,
0xC0, 0xC0,
0xD0, 0xD0,
0xE0, 0xE0,
0xF0, 0xF0
);

// Palette color band max values
// These are only somewhat correct ...
const int highs[32] = int[32](
0x2F, 0x2F, 0x2F, 0x2F, 0x2F, 0x2F,
0x5F, 0x5F, 0x5F, 0x5F, 0x5F, 0x5F,
0x7F, 0x7F, 0x7F, 0x7F,
0x9F, 0x9F, 0x9F, 0x9F,
0xAF, 0xAF,
0xBF, 0xBF,
0xCF, 0xCF,
0xDF, 0xDF,
0xEF, 0xEF,
0xFF, 0xFF
);

// In
in vec2 tex_coord;
layout (std140) uniform palette {
    vec4 colors[256];
};

uniform sampler2D framebuffer;
uniform sampler2D remaps;

// Out
layout (location = 0) out vec4 color;

// TODO: We can do this stuff with just floats.
// TODO: See if we can get rid of some of the ifs ?

void main() {
    vec4 texel = texture(framebuffer, tex_coord);
    float offset = 3.0 / 255.0;
    int add_remap = int(texture(remaps, vec2(texel.r, texel.g + offset)).r * 255.0);
    int sub_remap = int(texture(remaps, vec2(texel.r, texel.b + offset)).r * 255.0);
    int color_index = int(texel.r * 255.0);
    int add_index = int(texel.g * 255.0);
    int sub_index = int(texel.b * 255.0);

    if (color_index <= 0) {
        // Color index 0 is always magic black.
        color = colors[0].rgba;
    }
    else if (add_index > 0) {
        color = colors[add_remap].rgba;
    }
    else if (sub_index > 0) {
        color = colors[sub_remap].rgba;
    }
    else {
        // Normal draw; just pick the color and output it.
        color = colors[color_index].rgba;
    }
}
