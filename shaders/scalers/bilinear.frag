#version 330 core

// In
in vec2 tex_coord;
uniform sampler2D framebuffer;
uniform vec2 texture_size;

// Out
layout (location = 0) out vec4 color;

void main() {
    vec2 texel = tex_coord * texture_size - 0.5;
    vec2 texel_floor = floor(texel);
    vec2 texel_fract = fract(texel);
    vec2 texel_size = 1.0 / texture_size;

    // sample surrounding pixels
    vec4 c00 = texture(framebuffer, (texel_floor + vec2(0.0, 0.0)) * texel_size + 0.5 * texel_size);
    vec4 c10 = texture(framebuffer, (texel_floor + vec2(1.0, 0.0)) * texel_size + 0.5 * texel_size);
    vec4 c01 = texture(framebuffer, (texel_floor + vec2(0.0, 1.0)) * texel_size + 0.5 * texel_size);
    vec4 c11 = texture(framebuffer, (texel_floor + vec2(1.0, 1.0)) * texel_size + 0.5 * texel_size);

    vec4 c0 = mix(c00, c10, texel_fract.x);
    vec4 c1 = mix(c01, c11, texel_fract.x);
    color = mix(c0, c1, texel_fract.y);
}
