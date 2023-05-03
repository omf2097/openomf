#version 330 core

in vec2 tex_coord;
layout (location = 0) out vec4 color;
layout (std140) uniform props {
    int mode;
};

uniform sampler2D image;

void main() {
    vec4 texel = texture(image, tex_coord);
    if (texel.g == 0) {
        discard;
    }
    color = texel.rgba;
}
