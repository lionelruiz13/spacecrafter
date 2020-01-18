//
//	ART
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec2 position;
layout (location=1)in vec2 texCoord;

//~ uniform mat4 MVP;

layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};

out vec2 TexCoord;

//~ in Interpolators
//~ {
	//~ vec2 TexCoord;
//~ } interData;

void main()
{
	//~ gl_Position = MVP2D * vec4(position,0.0,1.0);
	TexCoord = texCoord;
	gl_Position = vec4(position,0.0,1.0);
	//~ interData.TexCoord = texCoord;
}

//~ #version 420
//~ #pragma debug(on)
//~ #pragma optimize(off)

//~ layout (location=0) in vec2 position;
//~ layout (location=1) in float intensity;
//~ layout (location=2) in vec2 taille;

//~ out float Intensity ;
//~ out vec2 Taille ;

//~ void main()
//~ {
	//~ Intensity = intensity;
	//~ Taille = taille;
	//~ gl_Position = vec4(position,0.0,1.0);
//~ }
