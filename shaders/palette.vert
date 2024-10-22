#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex_offset;
layout (location = 2) in int transparency;
layout (location = 3) in int r_offset;
layout (location = 4) in int r_rounds;
layout (location = 5) in int p_offset;
layout (location = 6) in int p_limit;
uniform mat4 projection;

out vec2 tex_coord;
flat out int transparency_index;
flat out int remap_offset;
flat out int remap_rounds;
flat out int palette_offset;
flat out int palette_limit;

void main() {
    transparency_index = transparency;
    palette_offset = p_offset;
    palette_limit = p_limit;
    remap_offset = r_offset;
    remap_rounds = r_rounds;
    tex_coord = tex_offset.xy;
    gl_Position = projection * vec4(position.xy, 0.0, 1.0);
}
