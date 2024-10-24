#version 330 core

layout (location = 0) out vec4 color;

in vec2 tex_coord;
flat in int transparency_index;
flat in int remap_offset;
flat in int remap_rounds;
flat in int palette_offset;
flat in int palette_limit;
flat in uint options;

uniform sampler2D atlas;
uniform sampler2D remaps;

uint use_sprite_remap = options & 1u;


vec4 handle(float index, float remap) {
    if (remap_rounds > 0) {
        float r_index = remap_offset / 255.0 + index;
        float r_rounds = remap_rounds / 255.0;
        return vec4(0.0, r_index, r_rounds, 1.0);
    }
    return vec4(index, 0.0, 0.0, 1.0);
}

void main() {
    vec4 texel = texture(atlas, tex_coord);
    vec4 remap = texture(remaps, vec2(texel.r, remap_offset / 18.0));

    // Don't render if it's transparent pixel
    int index = int(texel.r * 255.0);
    if (index == transparency_index)
    discard;

    // If remapping is on, do it now.
    if (use_sprite_remap == 1u)
    texel = remap;

    // Palette offset and limit (for e.g. fonts)
    float limit = palette_limit / 255.0;
    float offset = palette_offset / 255.0;
    if (texel.r <= limit) {
        texel.r = clamp(texel.r + offset, 0, limit);
    }

    color = handle(texel.r, remap.r);
}