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

// Atlas texture (GL_R8UI): uint8 palette index (0-255) per texel.
uniform usampler2D atlas;

// Remap texture (GL_R16UI, 1024x19): uint16 palette index (0-1023) per texel.
// X axis is the source palette index, Y axis selects one of 19 remap tables.
uniform usampler2D remaps;

in vec4 gl_FragCoord;

// for info about these options, see renderer_options in video/enums.h
bool SPRITE_REMAP = (options & 1u) != 0u;
bool SPRITE_SHADOWMASK = (options & 2u) != 0u;
bool SPRITE_INDEX_ADD = (options & 4u) != 0u;
bool SPRITE_HAR_QUIRKS = (options & 8u) != 0u;
bool SPRITE_DARK_TINT = (options & 0x10u) != 0u;

const int MAGIC_REMAP_ROUNDS = 12;
const float PHI = 1.61803398874989484820459;
const float ATLAS_H = 2048.0;
const vec2 NATIVE_SIZE = vec2(320.0, 200.0);

float noise(in vec2 v) {
    // OMF had a fairly random offset for each row, because
    // they applied their noise row by row.
    // they then changed the threshold by 0x6b for each subsequent X.
    return fract(tan(10 * PHI * v.y) + (float(0x6b) * v.x) / 256.0);
}

// Encode a palette index into the paletted framebuffer (GL_RGBA16).
// Channels are written selectively via glColorMask depending on blend mode:
//   MODE_SET:            RGBA  (base sprite)
//   MODE_DARK_TINT:      _GBA  (tint overlay)
//   MODE_REMAP:          _G__  (remap overlay, on top of existing R from MODE_SET)
//   MODE_SPRITE_SHADOW:  _G__  (shadow overlay, GL_MAX blended)
//   MODE_ADD:            ___A  (additive index overlay)
//
// Output channels:
//   R: palette index / 1023.0
//   G: (remap_offset + remap_rounds * 19 [+ index]) / 255.0 -- decoded by rgba.frag
//   B: dark tint palette index / 1023.0
//   A: additive index (index * 60) / 1023.0
vec4 handle(int index) {
    if (SPRITE_DARK_TINT) {
        // use magic rounds to detect if dark_tint's remap has been
        // overwritten by the pause menu's background
        float remap = float(remap_offset + MAGIC_REMAP_ROUNDS * 19) / 255.0;
        return vec4(0.0, remap, float(index) / 1023.0, 0.0);
    }
    if (remap_rounds > 0) {
        float remap = float(remap_offset + remap_rounds * 19 + index) / 255.0;
        return vec4(0.0, remap, 0.0, 0.0);
    }
    if (SPRITE_INDEX_ADD) {
        float add = float(index * 60) / 1023.0;
        return vec4(0.0, 0.0, 0.0, add);
    }
    return vec4(float(index) / 1023.0, 0.0, 0.0, 0.0);
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
            float offset = float(y - 1) / ATLAS_H;
            uvec4 texel = textureLod(atlas, tex_coord + vec2(0, offset), 0);
            int index = int(texel.r);
            coverage += int(index != transparency_index);
        }

        // don't draw transparent pixels
        if(coverage == 0) {
            discard;
        }

        color = handle(coverage);
        return;
    }


    uvec4 texel = textureLod(atlas, tex_coord, 0);

    // Don't render if it's transparent pixel
    int index = int(texel.r);
    if (index == transparency_index) {
        discard;
    }

    // Palette offset and limit (for e.g. fonts)
    if (index <= palette_limit) {
        index = clamp(index + palette_offset, 0, palette_limit);
    }

    bool NO_REMAP = SPRITE_HAR_QUIRKS && index > 0x30;

    // If remapping is on, do it now.
    if (SPRITE_REMAP && !NO_REMAP) {
        uvec4 remap = textureLod(remaps, vec2((float(index) + 0.5) / 1024.0, remap_offset / 18.0), 0);
        index = int(remap.r);
    }

    color = handle(index);
}
