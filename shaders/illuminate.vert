//
// illuminate
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec3 position;
layout (location=1)in vec2 texCoord;

layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};

smooth out vec2 TexCoord;


void main()
{
	gl_Position = MVP2D * vec4(position,1.0);
	TexCoord = texCoord;
}
