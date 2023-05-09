#version 330 core

in vec2 tex_coord;
in vec2 fbo_coord;
in float blend_mode;
layout (location = 0) out vec4 color;
uniform sampler2D atlas;
uniform sampler2D framebuffer;
layout (std140) uniform remaps {
    vec4 mappings[19 * 256];
};

void main() {
    // Get rid of the branching later! We can split this into two or three shaders pretty easily
    // TODO: FIX LATER.
    if (blend_mode < 0.5) {
        vec4 texel = texture(atlas, tex_coord);
        if (texel.g == 0) {
            discard;
        }
        vec4 src = texture(framebuffer, fbo_coord);
        float pal_index = src.r * 255.0;
        float sprite_index = texel.r * 255.0;
        int layer = int(clamp(18.9, 0.0, sprite_index + 3.0));
        int index = int(clamp(255.0, 0.0, pal_index));
        int offset = layer * 256 + index;
        float red = mappings[offset].r;
        color = vec4(red, 0.0, 0.0, 1.0);
    } else if (blend_mode < 1.5) {
        vec4 texel = texture(atlas, tex_coord);
        if (texel.g == 0) {
            discard;
        }
        color = texel.rgba;
    } else {
        color = texture(framebuffer, tex_coord).rgba;
    }
}
