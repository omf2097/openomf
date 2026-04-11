#version 330 core

in vec2 tex_coord;
layout (std140) uniform palette {
    vec4 colors[1024];
};

uniform usampler2D atlas;

layout (location = 0) out vec4 color;

void main() {
    uvec4 texel = texture(atlas, tex_coord);
    color = colors[int(texel.r)];
}
