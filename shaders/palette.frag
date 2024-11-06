#version 330 core

layout (location = 0) out vec4 color;

in vec2 tex_coord;
flat in int transparency_index;
flat in int remap_offset;
flat in int remap_rounds;
flat in int palette_offset;
flat in int palette_limit;
flat in int opacity;
flat in uint options;

uniform sampler2D atlas;
uniform sampler2D remaps;

in vec4 gl_FragCoord;

uint use_sprite_remap = options & 1u;
uint use_sprite_mask = options & 2u;
uint use_index_add = options & 4u;


float PHI = 1.61803398874989484820459;

float noise(in vec2 v) {
    return fract(tan(distance(v * PHI, v)) * v.x);
}

vec4 handle(float index, float remap) {
    if (remap_rounds > 0) {
        float r_index = remap_offset / 255.0 + index;
        float r_rounds = remap_rounds / 255.0;
        return vec4(0.0, r_index, r_rounds, 0.0);
    }
    if (use_index_add > 0u) {
        float add = (index * 255.0 * 60) / 255.0;
        return vec4(0.0, 0.0, 0.0, add);
    }
    return vec4(index, 0.0, 0.0, 0.0);
}

void main() {
    vec4 texel = texture(atlas, tex_coord);

    // Don't render if it's transparent pixel
    int index = int(texel.r * 255.0);
    if (index == transparency_index) {
        discard;
    }

    // Don't render if we're decimating due to opacity
    float decimate_limit = opacity / 255.0;
    float decimate_value = noise(gl_FragCoord.xy);
    if (decimate_value > decimate_limit) {
        discard;
    }

    // Palette offset and limit (for e.g. fonts)
    float limit = palette_limit / 255.0;
    float offset = palette_offset / 255.0;
    if (texel.r <= limit) {
        texel.r = clamp(texel.r + offset, 0, limit);
    }

    vec4 remap = texture(remaps, vec2(texel.r, remap_offset / 18.0));

    // If remapping is on, do it now.
    if (use_sprite_remap > 0u) {
        texel = remap;
    }

    // If masking is on, set our color to always be index 1.
    if (use_sprite_mask > 0u) {
        texel.r = 1.0 / 255.0;
    }

    color = handle(texel.r, remap.r);
}