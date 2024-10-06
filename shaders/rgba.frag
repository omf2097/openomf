#version 330 core

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
    int color_index = int(texel.r * 255.0);
    int add_index = int(texel.g * 255.0);
    int real_index = color_index + add_index;
    if (color_index <= 0) {
        color = colors[0].rgba;
    }
    else if (add_index > 0) {
        float row = floor(color_index / 16.0);
        int high = int(row * 16.0) + 16;
        if (real_index >= high) {
            color = colors[0xEF].rgba;
        } else {
            color = colors[real_index].rgba;
        }
    }
    else {
        color = colors[color_index].rgba;
    }
}
