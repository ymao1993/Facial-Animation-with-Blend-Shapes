#version 430 core

in vec2 txcoord;
uniform sampler2D sampler;

vec4 texelFetch(sampler2D tex, ivec2 size, ivec2 coord);

out vec4 color;

void main(void)
{
    color =  texture2D(sampler,txcoord);
}

