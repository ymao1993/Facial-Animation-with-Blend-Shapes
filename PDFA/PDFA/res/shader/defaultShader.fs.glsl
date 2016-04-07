#version 430 core

in vec2 txcoord;
in vec3 w_position;
in vec3 w_normal;

//texture mapping
uniform sampler2D sampler;
uniform bool hasTexture = false;

//simple lighting
uniform vec3 eyepos;
uniform vec3 light;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float delta;

//output color
out vec4 color;

void main(void)
{
	if(hasTexture)
	{
		color =  texture2D(sampler,txcoord);
	}
	else
	{
		//apply phong shading...
		color = vec4(ambient,1);
		color += vec4(diffuse * dot(w_normal,-light),1);
		color += vec4(specular * pow(max(0,dot(normalize(w_position - eyepos), reflect(-light, w_normal))),delta) ,1);
	}
}

