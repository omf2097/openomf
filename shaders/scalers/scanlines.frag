#version 330 core

// CRT colors with scanlines
// The ideas used here are mostly from https://github.com/libretro/slang-shaders/
// and https://github.com/dosbox-staging/dosbox-staging shaders.

// Tunables
uniform float scanline_intensity = 0.4;
uniform float scanlines_per_row = 2.0;
uniform float beam_width = 0.4;
const int beam_taps = 2;
const float beam_tap_step = 0.5;
const float source_gamma = 2.4;
const float display_gamma = 2.2;
const float native_width = 320.0;
const float native_height = 200.0;
const float min_blend_width = 1.0 / 256.0;

// In
in vec2 tex_coord;
uniform vec2 texture_size;
uniform sampler2D framebuffer;

// Out
layout (location = 0) out vec4 color;

const float pi = 3.14159265;

// display to linear light
vec3 to_linear(vec3 c) {
    return pow(c, vec3(source_gamma));
}

// linear to display light
vec3 to_display(vec3 c) {
    return pow(clamp(c, 0.0, 1.0), vec3(1.0 / display_gamma));
}

// fetch and convert to linear light
vec4 fetch_linear(vec2 texel) {
    vec4 c = texture(framebuffer, texel / texture_size);
    return vec4(to_linear(c.rgb), c.a);
}

vec4 sample_bilinear(vec2 texel_floor, vec2 texel_fract) {
    vec4 c00 = fetch_linear(texel_floor + vec2(0.5, 0.5));
    vec4 c10 = fetch_linear(texel_floor + vec2(1.5, 0.5));
    vec4 c01 = fetch_linear(texel_floor + vec2(0.5, 1.5));
    vec4 c11 = fetch_linear(texel_floor + vec2(1.5, 1.5));
    vec4 top = mix(c00, c10, texel_fract.x);
    vec4 bottom = mix(c01, c11, texel_fract.x);
    return mix(top, bottom, texel_fract.y);
}

void main() {
    // Position in framebuffer pixel space, offset so that pixel centers land on integers
    vec2 texel = tex_coord * texture_size - 0.5;
    vec2 texel_floor = floor(texel);
    vec2 texel_fract = fract(texel);

    // Sharp bilinear
    vec2 texels_per_pixel = clamp(fwidth(texel), min_blend_width, 1.0);
    vec2 sharp_fract = clamp((texel_fract - 0.5) / texels_per_pixel + 0.5, 0.0, 1.0);

    float tap_step = beam_tap_step * texture_size.x / native_width;
    vec4 center = sample_bilinear(texel_floor, sharp_fract);
    vec3 sum = center.rgb;
    float weight_sum = 1.0;
    for(int i = 1; i <= beam_taps; i++) {
        float dist = float(i) * beam_tap_step;
        float weight = exp(-0.5 * dist * dist / (beam_width * beam_width));
        vec2 offset = vec2(float(i) * tap_step, 0.0);
        sum += weight * sample_bilinear(texel_floor - offset, sharp_fract).rgb;
        sum += weight * sample_bilinear(texel_floor + offset, sharp_fract).rgb;
        weight_sum += 2.0 * weight;
    }
    vec3 linear_color = sum / weight_sum;

    // Scanline profile
    float line_pos = tex_coord.y * native_height * scanlines_per_row - 0.25;
    float gap_darkness = 0.5 + 0.5 * cos(2.0 * pi * line_pos);

    // How many scanlines a screen pixel covers
    float lines_per_pixel = clamp(fwidth(line_pos), min_blend_width, 1.0);

    // Fade the effect out smoothly instead of aliasing into moire
    float attenuation = sin(pi * lines_per_pixel) / (pi * lines_per_pixel);

    // Darken the gaps multiplicatively so shadow detail survives
    float scanline_factor = 1.0 - scanline_intensity * gap_darkness * attenuation;

    // Boost gap brightness so the mean brightness stays as it was
    float brightness_compensation = 1.0 / (1.0 - 0.5 * scanline_intensity * attenuation);
    linear_color *= min(scanline_factor * brightness_compensation, 1.0);

    color = vec4(to_display(linear_color), center.a);
}
