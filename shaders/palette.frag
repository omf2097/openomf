#version 330 core

layout (location = 0) out vec4 color;

in vec2 tex_coord;
flat in int transparency_index;
flat in int blend_mode;
flat in int palette_offset;
flat in int palette_limit;

uniform sampler2D atlas;

#define BLEND_SET 0
#define BLEND_ADD 1
#define BLEND_SUB 2

vec4 handle(float index) {
    switch(blend_mode) {
        case BLEND_SET: return vec4(index, 0.0, 0.0, 1.0);
        case BLEND_ADD: return vec4(0.0, index, 0.0, 1.0);
        case BLEND_SUB: return vec4(0.0, 0.0, index, 1.0);
    }
}

void main() {
    vec4 texel = texture(atlas, tex_coord);

    // Don't render if it's transparent pixel
    int index = int(texel.r * 255.0);
    if (index == transparency_index) discard;

    // Palette offset and limit (for e.g. fonts)
    float limit = palette_limit / 255.0;
    float offset = palette_offset / 255.0;
    if (texel.r <= limit) {
        texel.r = clamp(texel.r + offset, 0, limit);
    }

    color = handle(texel.r);
}