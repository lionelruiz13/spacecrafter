//
// fbo
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;

layout (location=0) in vec2 TexCoord;
 
layout (location=0) out vec4 FragColor;
 
void main(void)
{
	FragColor = vec4(vec3(texture(mapTexture,TexCoord)).rgb, 1.0);
}

