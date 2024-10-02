#version 330 core

layout (location = 0) out vec4 color;

in vec2 tex_coord;
flat in int blend_mode;
flat in int palette_offset;
flat in int palette_limit;
flat in int alternate_index;

uniform sampler2D atlas;

vec4 handle(float index, float limit, float offset) {
    switch(blend_mode) {
        case 0: return vec4(0.0, index, 0.0, 1.0);  // ADD
        case 1: return vec4(0.0, 0.0, index, 1.0);  // SUB
        case 2: return vec4(index, 0.0, 0.0, 1.0);  // SET
    }
}

void main() {
    vec4 texel = texture(atlas, tex_coord);
    float index = texel.r;
    float opacity = texel.g;
    float limit = palette_limit / 255.0;
    float offset = palette_offset / 255.0;

    // Don't render if it's transparent pixel
    if (opacity == 0) discard;

    // Palette offset and limit (for e.g. fonts)
    if (index <= limit) {
        index = clamp(limit, 0, index + offset);
    }

    // Instead of using texture index, use alternate index instead.
    if(alternate_index > 0) {
        index = alternate_index;
    }

    color = handle(index, limit, offset);
}
