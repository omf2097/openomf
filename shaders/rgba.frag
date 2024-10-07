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

void main() {
    vec4 texel = texture(framebuffer, tex_coord);
    int add_remap = int(round(texture(remaps, vec2(texel.r, texel.g)).r * 255.0));
    int sub_remap = int(round(texture(remaps, vec2(texel.r, texel.b)).r * 255.0));
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