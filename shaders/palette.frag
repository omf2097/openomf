#version 330 core

layout (location = 0) out vec4 color;

in vec2 tex_coord;
in vec2 fbo_coord;
flat in int blend_mode;
flat in int palette_offset;
flat in int palette_limit;

uniform sampler2D atlas;
uniform sampler2D framebuffer;
layout (std140) uniform remaps {
    vec4 mappings[19 * 256];
};

void main() {
    // Get rid of the branching later! We can split this into two or three shaders pretty easily
    // TODO: FIX LATER.
    if (blend_mode == 0) {  // Additive
                            vec4 texel = texture(atlas, tex_coord);
                            if (texel.g == 0) {
                                discard;
                            }
                            vec4 src = texture(framebuffer, fbo_coord);
                            int pal_index = int(src.r * 256.0);
                            if (pal_index <= palette_limit) {
                                pal_index = clamp(palette_limit, 0, pal_index + palette_offset);
                            }
                            float sprite_index = texel.r * 255.0;
                            int layer = int(clamp(18.9, 0.0, sprite_index + 3.0));
                            int index = clamp(255, 0, pal_index);
                            int offset = layer * 256 + index;
                            float red = mappings[offset].r;
                            color = vec4(red, 0.0, 0.0, 1.0);
    } else if (blend_mode == 1) {  // Alpha
                                   vec4 texel = texture(atlas, tex_coord);
                                   if (texel.g == 0) {
                                       discard;
                                   }
                                   int pal_index = int(texel.r * 256.0);
                                   if (pal_index <= palette_limit) {
                                       pal_index = clamp(palette_limit, 0, pal_index + palette_offset);
                                   }
                                   color = vec4(pal_index / 255.0, 0.0, 0.0, 1.0);
    } else {
        color = texture(framebuffer, tex_coord).rgba;
    }
}
