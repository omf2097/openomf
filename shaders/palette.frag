#version 330 core

in vec2 tex_coord;
layout (location = 0) out vec4 color;

uniform sampler2D image;

void main() {
    vec4 texel = texture(image, tex_coord);
    if (texel.g == 0) {
        discard;
    }
    color = texel.rgba;
}
