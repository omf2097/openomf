#version 330 core

// In
in vec2 tex_coord;
layout (std140) uniform palette {
    vec4 colors[256];
};
uniform sampler2D image;

// Out
layout (location = 0) out vec4 color;

void main() {
    vec4 texel = texture(image, tex_coord);
    int index = int(texel.r * 255);
    color = colors[index].rgba;
}
