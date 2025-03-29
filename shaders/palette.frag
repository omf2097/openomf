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
uniform sampler2D loopback;

in vec4 gl_FragCoord;

bool SPRITE_REMAP = (options & 1u) != 0u;
bool SPRITE_SHADOWMASK = (options & 2u) != 0u;
bool SPRITE_INDEX_ADD = (options & 4u) != 0u;
bool SPRITE_HAR_QUIRKS = (options & 8u) != 0u;
bool SPRITE_LOOPBACK = (options & 0x10u) != 0u;


float PHI = 1.61803398874989484820459;
float ATLAS_H = 2048.0;
vec2 NATIVE_SIZE = vec2(320.0, 200.0);

float noise(in vec2 v) {
    return fract(tan(distance(v * PHI, v)) * v.x);
}

vec4 handle(float index) {
    if (remap_rounds > 0) {
        float remap = float(remap_offset) / 255.0 + index
            + float(remap_rounds) * 19.0 / 255.0;
        return vec4(0.0, remap, 0.0, 0.0);
    }
    if (SPRITE_INDEX_ADD) {
        float add = (index * 255.0 * 60) / 255.0;
        return vec4(0.0, 0.0, 0.0, add);
    }
    return vec4(index, 0.0, 0.0, 0.0);
}

void main() {
    // Don't render if we're decimating due to opacity
    float decimate_limit = opacity / 255.0;
    float decimate_value = noise(gl_FragCoord.xy);
    if (decimate_value > decimate_limit) {
        discard;
    }

    if (SPRITE_SHADOWMASK) {
        // make four samples to generate coverage
        int coverage = 0;
        for(int y = 0; y < 4; y++) {
            float offset = float(y) / ATLAS_H;
            vec4 texel = texture(atlas, tex_coord + vec2(0, offset));
            int index = int(texel.r * 255.0);
            coverage += int(index != transparency_index);
        }

        // don't draw transparent pixels
        if(coverage == 0) {
            discard;
        }

        color = handle(float(coverage) / 256.0);
        return;
    }


    vec4 texel = texture(atlas, tex_coord);

    // Don't render if it's transparent pixel
    int index = int(texel.r * 255.0);
    if (index == transparency_index) {
        discard;
    }

    // Palette offset and limit (for e.g. fonts)
    float limit = palette_limit / 255.0;
    float offset = palette_offset / 255.0;
    if (texel.r <= limit) {
        texel.r = clamp(texel.r + offset, 0, limit);
    }

    bool NO_REMAP = SPRITE_HAR_QUIRKS && index > 0x30;

    // chronos's stasis & shadow's shadow transparency
    if(SPRITE_LOOPBACK) {
        index = int(texel.r * 255.0);
        if(index < 0x60) {
            // grab color index we're drawing ontop of
            int behind = int(texture(loopback, gl_FragCoord.xy / NATIVE_SIZE).r * 255.0);
            // lookup behind in remap 5 to get brightness
            vec4 remap = texture(remaps, vec2(float(behind) / 255.0, 4.0 / 18.0));
            behind = 1 + clamp(int(remap.r * 255.0) - 0xA8, 0, 7) * 2;

            // this indexed blending is not exactly correct, but i am
            // done fiddling with it for now-- if someone finds the
            // code for this in MASTER.DAT, then we can correct it.
            index = (index & 0xF0) + ((index & 0x0F) * 3 + behind * 2) / 5;
        }
        texel.r = float(index) / 255.0;
    }

    // If remapping is on, do it now.
    if (SPRITE_REMAP && !NO_REMAP) {
        vec4 remap = texture(remaps, vec2(texel.r, remap_offset / 18.0));

        texel = remap;
    }

    color = handle(texel.r);
}
