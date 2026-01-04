#version 330 core

// Note: This is modified from https://godotshaders.com/shader/very-simple-crt-shader/

uniform float scanline_intensity = 0.25;
uniform float color_bleed_weight = 0.35;

// In
in vec2 tex_coord;
uniform vec2 texture_size;
uniform sampler2D framebuffer;

// Out
layout (location = 0) out vec4 color;

void main() {
    // Darken top halves of pixels
    float color_dark_offset = 0.0;
    int y_pos = int(floor(tex_coord.y * texture_size.y * 2.0));
    if (int(floor((float(y_pos) / 2.0))) * 2 == y_pos) {
        color_dark_offset = scanline_intensity;
    }

    // Blend pixel with left and top pixel to simulate color bleeding
    vec4 adjacent_pixel_color_average = texture(framebuffer, tex_coord - vec2(1.0 / texture_size.x, 0)) * 0.5 + texture(framebuffer, tex_coord - vec2(0, 1.0 / texture_size.y)) * 0.5;
    vec4 this_pixel_color = texture(framebuffer, tex_coord);

    color = adjacent_pixel_color_average * color_bleed_weight + this_pixel_color * (1.0 - color_bleed_weight) - vec4(vec3(color_dark_offset), 0);
}
