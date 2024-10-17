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

const float offset = 3 / 19.0;

void main() {
    vec4 texel = texture(framebuffer, tex_coord);
    vec4 tex_add = texture(remaps, vec2(texel.r, texel.g * 19.0 + offset));
    vec4 tex_sub = texture(remaps, vec2(texel.r, texel.b * 19.0 + offset));

    int out_index = int(texel.r * 255.0);
    if (texel.g > 0) {
        out_index = int(tex_add.r * 255.0);
    }
    else if (texel.b > 0) {
        out_index = int(tex_sub.r * 255.0);
    }
    color = colors[out_index];
}