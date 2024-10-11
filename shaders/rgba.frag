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

const float offset = 4;

void main() {
    vec4 texel = texture(framebuffer, tex_coord);
    int add_index = clamp(int(texel.g * 255.0), 0, 19);
    int sub_index = clamp(int(texel.b * 255.0), 0, 19);
    vec4 tex_add = texture(remaps, vec2(texel.r, (add_index + offset) / 19.0));
    vec4 tex_sub = texture(remaps, vec2(texel.r, (sub_index + offset) / 19.0));

    if (add_index > 0) {
        color = colors[int(tex_add.r * 255.0)].rgba;
    }
    else if (sub_index > 0) {
        color = colors[int(tex_sub.r * 255.0)].rgba;
    }
    else {
        // Normal draw; just pick the color and output it.
        color = colors[int(texel.r * 255.0)].rgba;
    }
}