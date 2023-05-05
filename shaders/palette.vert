#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 tex_offset;
layout (location = 2) in float mode;
uniform mat4 projection;

out vec2 tex_coord;
out vec2 fbo_coord;
out float blend_mode;

void main() {
    blend_mode = mode;
    tex_coord = tex_offset.xy;
    fbo_coord = vec2(
        position.x / 320.0,
        (200.0 - position.y) / 200.0
    );
    gl_Position = projection * vec4(position.xy, 0.0, 1.0);
}
