#version 430 core


in vec2 txcoord;
in vec4 vs_color;

uniform sampler2D sampler;
uniform bool hasTexture = false;

uniform vec3 lightDir;




out vec4 color;

void main(void)
{
	if(hasTexture)
	{
		color =  vs_color + texture2D(sampler,txcoord);
	}
	else
	{
		color = vs_color;
	}
}

