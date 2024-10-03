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
// Out
layout (location = 0) out vec4 color;

// TODO: We can do this stuff with just floats.
// TODO: See if we can get rid of some of the ifs ?

void main() {
    vec4 texel = texture(framebuffer, tex_coord);
    int color_index = int(texel.r * 255.0);
    int add_index = int((texel.g * 255.0) * 255.0 / 63.0);  // Palette 6bit to 8bit conversion + float2int conversion.
    int sub_index = int((texel.b * 255.0) * 255.0 / 63.0);  // Palette 6bit to 8bit conversion + float2int conversion.

    if (color_index <= 0) {
        // Color index 0 is always magic black.
        color = colors[0].rgba;
    }
    else if (add_index > 0) {
        // If additive value is set, check if we are within a color slide. If we go outside,
        // then just use the 0xEF or "white" color.
        int real_index = color_index + add_index;
        int row = int(color_index / 8.0);
        if (real_index > highs[row]) {
            color = colors[0xEF].rgba;
        } else {
            color = colors[real_index].rgba;
        }
    }
    else if (sub_index > 0) {
        // If subtractive value is set, check if we are within a color slide.
        int row = int(color_index / 8.0);
        int real_index = clamp(color_index - sub_index, lows[row], 255);
        color = colors[real_index].rgba;
    }
    else {
        // Normal draw; just pick the color and output it.
        color = colors[color_index].rgba;
    }
}
