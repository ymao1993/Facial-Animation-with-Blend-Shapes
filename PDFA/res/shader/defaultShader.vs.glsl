#version 120

uniform mat4 m2v;
uniform mat4 persp;

attribute vec3 vs_position;
attribute vec2 vs_texcoord;

varying vec2 txcoord;

void main(void)
{
    gl_Position = persp * m2v * vec4(vs_position,1);
    txcoord = vs_texcoord;
}