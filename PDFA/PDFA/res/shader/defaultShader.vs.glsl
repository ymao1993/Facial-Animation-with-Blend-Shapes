#version 430 core

uniform mat4 m2w;
uniform mat4 w2v;
uniform mat4 persp;

//simple lighting
uniform vec3 eyepos;
uniform vec3 light;
uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform float delta;

//neutral attributes
in vec3 vs_position;
in vec2 vs_texcoord;
in vec3 vs_norm;


#define NUM_BLENDINGSHAPE 6
//morphing targets 
//we cannot use more than 6 morph 
//targets because of the limit of 
//max vertex attributes number, 
//which is 16
in vec3 pos0;
in vec3 norm0;
in vec3 pos1;
in vec3 norm1;
in vec3 pos2;
in vec3 norm2;
in vec3 pos3;
in vec3 norm3;
in vec3 pos4;
in vec3 norm4;
in vec3 pos5;
in vec3 norm5;

out vec2 txcoord;
out vec4 vs_color;

uniform float weights[NUM_BLENDINGSHAPE];
uniform samplerBuffer bs_sampler;

void main(void)
{
    txcoord = vs_texcoord;
    
    vec3 blended_pos = vs_position;
	vec3 blended_norm = vs_norm;

	//blending position...
	blended_pos += pos0  * weights[0];
	blended_pos += pos1  * weights[1];
	blended_pos += pos2  * weights[2];
	blended_pos += pos3  * weights[3];
	blended_pos += pos4  * weights[4];
	blended_pos += pos5  * weights[5];
    vec4 position = m2w * vec4(blended_pos,1);
	position.x /= position.w;
	position.y /= position.w;
	position.z /= position.w;
	position.w = 1;

	//blending normal...
	blended_norm += norm0 * weights[0];
	blended_norm += norm1 * weights[1];
	blended_norm += norm2 * weights[2];
	blended_norm += norm3 * weights[3];
	blended_norm += norm4 * weights[4];
	blended_norm += norm5 * weights[5];

	//normalize
	blended_norm = normalize(blended_norm);

	//apply gouraud shading...
	vec3 wnorm = normalize((m2w*vec4(blended_norm,0)).xyz);
	vs_color = vec4(ambient,1);
	vs_color += vec4(diffuse * dot(wnorm,-light),1);
	vs_color += vec4(specular * pow(max(0,dot(normalize(position.xyz - eyepos), reflect(-light, wnorm))),delta) ,1);

	gl_Position = persp * w2v * position;

}