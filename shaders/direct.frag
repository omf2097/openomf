#version 330 core

in vec2 tex_coord;
layout (location = 0) out vec4 color;

uniform sampler2D image;

void main() {
    vec4 index = texture(image, tex_coord);
    if (index.g == 0) {
        discard;
    }
    color = vec4(index.r, 0.0, 0.0, 1.0);
}
