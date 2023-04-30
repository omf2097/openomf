#version 330 core

in vec2 tex_coord;
layout (location = 0) out vec4 color;
layout (std140) uniform palette {
    vec4 colors[256];
};

uniform sampler2D image;

void main() {
    vec4 texel = texture(image, tex_coord);
    if (texel.g == 0) {
        discard;
    }
    color = colors[int(texel.r * 255)];
}
