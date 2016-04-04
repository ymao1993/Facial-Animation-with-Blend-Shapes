#version 430 core

uniform mat4 m2v;
uniform mat4 persp;

//neutral attributes
in vec3 vs_position;
in vec2 vs_texcoord;

//morphing targets
in vec3 pos0;
in vec3 pos1;
in vec3 pos2;
in vec3 pos3;
in vec3 pos4;
in vec3 pos5;
in vec3 pos6;
in vec3 pos7;
in vec3 pos8;
in vec3 pos9;
in vec3 pos10;
in vec3 pos11;
in vec3 pos12;
in vec3 pos13;


out vec2 txcoord;

#define NUM_BLENDINGSHAPE 14
uniform float weights[NUM_BLENDINGSHAPE];
uniform samplerBuffer bs_sampler;

void main(void)
{
    txcoord = vs_texcoord;
    
    vec3 blended_pos = vs_position;

	//blending...
	blended_pos += pos0  * weights[0];
	blended_pos += pos1  * weights[1];
	blended_pos += pos2  * weights[2];
	blended_pos += pos3  * weights[3];
	blended_pos += pos4  * weights[4];
	blended_pos += pos5  * weights[5];
	blended_pos += pos6  * weights[6];
	blended_pos += pos7  * weights[7];
	blended_pos += pos8  * weights[8];
	blended_pos += pos9  * weights[9];
	blended_pos += pos10 * weights[10];
	blended_pos += pos11 * weights[11];
	blended_pos += pos12 * weights[12];
	blended_pos += pos13 * weights[13];
    
    gl_Position = persp * m2v * vec4(blended_pos,1);
}