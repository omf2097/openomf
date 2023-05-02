#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex_offset;
uniform mat4 projection;

out vec2 tex_coord;

void main() {
    tex_coord = tex_offset.xy;
    gl_Position = projection * vec4(position.xy, 0.0, 1.0);
}
