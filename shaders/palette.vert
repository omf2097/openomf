#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex_offset;
layout (location = 2) in int mode;
layout (location = 3) in int pal_offset;
layout (location = 4) in int pal_limit;
uniform mat4 projection;

out vec2 tex_coord;
flat out int blend_mode;
flat out int palette_offset;
flat out int palette_limit;

void main() {
    blend_mode = mode;
    palette_offset = pal_offset;
    palette_limit = pal_limit;
    tex_coord = tex_offset.xy;
    gl_Position = projection * vec4(position.xy, 0.0, 1.0);
}
