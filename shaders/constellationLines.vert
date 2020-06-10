//
//	CONSTELLATION_LINES
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0)in vec2 position;
layout (location=3)in vec4 color;

//externe
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

out vec4 Color;

void main()
{
	Color = color;
	gl_Position = MVP2D * vec4(position,0.0,1.0);
}

