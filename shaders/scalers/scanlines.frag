#version 330 core

// CRT colors with scanlines
// The ideas used here are mostly from https://github.com/libretro/slang-shaders/
// and https://github.com/dosbox-staging/dosbox-staging shaders.

// Tunables
uniform float scanline_intensity = 0.25;
uniform float color_bleed_weight = 0.35;
const float color_bleed_spread = 1.0;
const float source_gamma = 2.4;
const float display_gamma = 2.2;
const float native_height = 200.0;
const float min_blend_width = 1.0 / 256.0;

// In
in vec2 tex_coord;
uniform vec2 texture_size;
uniform sampler2D framebuffer;

// Out
layout (location = 0) out vec4 color;

const float pi = 3.14159265;

vec4 sample_bilinear(vec2 texel_floor, vec2 texel_fract) {
    vec2 texel_size = 1.0 / texture_size;
    vec4 c00 = texture(framebuffer, (texel_floor + vec2(0.5, 0.5)) * texel_size);
    vec4 c10 = texture(framebuffer, (texel_floor + vec2(1.5, 0.5)) * texel_size);
    vec4 c01 = texture(framebuffer, (texel_floor + vec2(0.5, 1.5)) * texel_size);
    vec4 c11 = texture(framebuffer, (texel_floor + vec2(1.5, 1.5)) * texel_size);
    vec4 top = mix(c00, c10, texel_fract.x);
    vec4 bottom = mix(c01, c11, texel_fract.x);
    return mix(top, bottom, texel_fract.y);
}

// dispaly to linear light
vec3 to_linear(vec3 c) {
    return pow(c, vec3(source_gamma));
}

// linear to display light
vec3 to_display(vec3 c) {
    return pow(clamp(c, 0.0, 1.0), vec3(1.0 / display_gamma));
}

void main() {
    // Position in framebuffer pixel space, offset so that pixel centers land on integers
    vec2 texel = tex_coord * texture_size - 0.5;
    vec2 texel_floor = floor(texel);
    vec2 texel_fract = fract(texel);

    // Sharp bilinear
    vec2 texels_per_pixel = clamp(fwidth(texel), min_blend_width, 1.0);
    vec2 sharp_fract = clamp((texel_fract - 0.5) / texels_per_pixel + 0.5, 0.0, 1.0);

    // Fetch this pixel and its neighbors
    vec2 bleed_offset = vec2(color_bleed_spread, 0.0);
    vec4 center = sample_bilinear(texel_floor, sharp_fract);
    vec4 left = sample_bilinear(texel_floor - bleed_offset, sharp_fract);
    vec4 right = sample_bilinear(texel_floor + bleed_offset, sharp_fract);

    // Color bleed
    vec3 linear_center = to_linear(center.rgb);
    vec3 linear_neighbors = (to_linear(left.rgb) + to_linear(right.rgb)) * 0.5;
    vec3 linear_color = mix(linear_center, linear_neighbors, color_bleed_weight);

    // Scanline profile
    float line_pos = tex_coord.y * native_height;
    float gap_darkness = 0.5 + 0.5 * cos(2.0 * pi * line_pos);

    // How many source lines a screen pixel covers
    float lines_per_pixel = clamp(fwidth(line_pos), min_blend_width, 1.0);

    // Fade the effect out smoothly instead of aliasing into moire
    float attenuation = sin(pi * lines_per_pixel) / (pi * lines_per_pixel);

    // Darken the gaps multiplicatively so shadow detail survives
    float scanline_factor = 1.0 - scanline_intensity * gap_darkness * attenuation;

    // Boost gap brightness so the mean brightness stays as it was
    float brightness_compensation = 1.0 / (1.0 - 0.5 * scanline_intensity * attenuation);

    linear_color *= scanline_factor * brightness_compensation;

    color = vec4(to_display(linear_color), center.a);
}
