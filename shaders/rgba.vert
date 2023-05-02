#version 330 core

const vec2 posCoord[4] = vec2[](vec2(320.0, 200.0), vec2(0.0, 200.0), vec2(0.0, 0.0), vec2(320.0, 0.0));
const vec2 texCoord[4] = vec2[](vec2(1.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 1.0), vec2(1.0, 1.0));

out vec2 tex_coord;
uniform mat4 projection;

void main() {
    tex_coord = texCoord[gl_VertexID].xy;
    gl_Position = projection * vec4(posCoord[gl_VertexID].xy, 0.0, 1.0);
}
