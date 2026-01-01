#version 330 core

// In
in vec2 tex_coord;
uniform vec2 texture_size;
uniform sampler2D framebuffer;

// Out
layout (location = 0) out vec4 color;

void main() {
    // This is a simple passthrough.
    color = texture(framebuffer, tex_coord);
}
