#version 330 core

// In
in vec2 tex_coord;
layout (std140) uniform palette {
    vec4 colors[256];
};

uniform sampler2D framebuffer;

// Out
layout (location = 0) out vec4 color;

void main() {
    vec4 texel = texture(framebuffer, tex_coord);
    color = colors[int(255.0 * texel.r)];
}
