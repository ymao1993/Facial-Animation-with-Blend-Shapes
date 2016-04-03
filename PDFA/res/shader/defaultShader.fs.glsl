#version 120

varying vec2 txcoord;
uniform sampler2D sampler;


vec4 texelFetch(sampler2D tex, ivec2 size, ivec2 coord);

void main(void)
{
    gl_FragColor = texture2D(sampler,txcoord);
}

