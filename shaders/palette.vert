#version 330 core

layout (location = 0) in vec2 in_position;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in int in_transparency_index;
layout (location = 3) in int in_remap_offset;
layout (location = 4) in int in_remap_rounds;
layout (location = 5) in int in_palette_offset;
layout (location = 6) in int in_palette_limit;
layout (location = 7) in int in_opacity;
layout (location = 8) in uint in_options;
uniform mat4 projection;

out vec2 tex_coord;
flat out int transparency_index;
flat out int remap_offset;
flat out int remap_rounds;
flat out int palette_offset;
flat out int palette_limit;
flat out int opacity;
flat out uint options;

void main() {
    transparency_index = in_transparency_index;
    palette_offset = in_palette_offset;
    palette_limit = in_palette_limit;
    remap_offset = in_remap_offset;
    remap_rounds = in_remap_rounds;
    opacity = in_opacity;
    options = in_options;
    tex_coord = in_tex_coord.xy;
    gl_Position = projection * vec4(in_position.xy, 0.0, 1.0);
}
