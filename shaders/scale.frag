#version 330 core

// In
in vec2 tex_coord;
uniform sampler2D framebuffer;

// Out
layout (location = 0) out vec4 color;

void main() {
    // This is a simple passthrough. When mode is nearest or bilinear, we let opengl do the filtering.
    // For custom modes, we need to add new shaders which are run instead of this.
    color = texture(framebuffer, tex_coord);
}
