#version 330 core

layout (location = 0) in vec4 vert;
uniform mat4 projection;

out vec2 TexCoords;

void main() {
    TexCoords = vert.zw;
    gl_Position = projection * vec4(vert.xy, 0.0, 1.0);
}
