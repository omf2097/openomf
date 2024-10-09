#version 330 core

layout (location = 0) out vec4 color;

in vec2 tex_coord;
flat in int transparency_index;
flat in int blend_mode;
flat in int palette_offset;
flat in int palette_limit;

uniform sampler2D atlas;

void main() {
    vec4 texel = texture(atlas, tex_coord);
    int pal_index = int(texel.r * 255.0);
    if (pal_index == transparency_index) discard;  // Don't render if it's transparent pixel
    if (blend_mode == 0) {
        color = vec4(0.0, texel.r, 0.0, 1.0);
    } else {
        if (pal_index <= palette_limit) {
            pal_index = clamp(palette_limit, 0, pal_index + palette_offset);
        }
        float float_index = pal_index / 255.0;
        color = vec4(float_index, 0.0, 0.0, 1.0);
    }
}
