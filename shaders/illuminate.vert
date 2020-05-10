//
// illuminate
//

#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (location=0)in vec3 position;
layout (location=1)in vec2 texCoord;
layout (location=2)in vec3 texColor;

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
smooth out vec3 TexColor;

void main()
{
	gl_Position = MVP2D * vec4(position,1.0);
	TexCoord = texCoord;
	TexColor = texColor;
}
