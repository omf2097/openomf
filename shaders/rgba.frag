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

float get_remapping_index(float src) {
    int remap_index = int(round(src * 255.0));
    int clamped_index = clamp(remap_index, 0, 18);
    return clamped_index / 18.0;
}

void main() {
    vec4 texel = texture(framebuffer, tex_coord);
    int remap_count = int(texel.b * 255.0);
    float remap_index = get_remapping_index(texel.g);

    // TODO: precalculate palette to 2d texture for required remappings later.
    for (int i = 0; i < remap_count; i++) {
        texel = texture(remaps, vec2(texel.r, remap_index));
    }

    color = colors[int(255.0 * texel.r)];
}