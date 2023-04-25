#version 330 core

in vec2 LVertexPos2D;

void main() {
    gl_Position = vec4(LVertexPos2D.x, LVertexPos2D.y, 0, 1);
}
