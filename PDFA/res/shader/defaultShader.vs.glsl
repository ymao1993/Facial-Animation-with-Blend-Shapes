#version 120
#extension GL_EXT_gpu_shader4 : require

uniform mat4 m2v;
uniform mat4 persp;

attribute vec3 vs_position;
attribute vec2 vs_texcoord;

varying vec2 txcoord;

#define NUM_BLENDINGSHAPE 14
uniform float weights[NUM_BLENDINGSHAPE];
uniform samplerBuffer bs_sampler;

void main(void)
{
    txcoord = vs_texcoord;
    
    vec3 blended_pos = vs_position;
    
    for(int i=0; i<NUM_BLENDINGSHAPE; i++)
    {
        //blended_pos += texelFetchBuffer(bs_sampler, NUM_BLENDINGSHAPE * gl_VertexID + i).xyz * weights[i];
    }
    
    gl_Position = persp * m2v * vec4(blended_pos,1);
}