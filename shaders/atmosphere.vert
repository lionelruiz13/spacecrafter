//
//	atmosphere
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec2 position;
layout (location=3)in vec3 color;

//out
smooth out vec3 Color;

//Externe
//~ uniform mat4 ModelViewProjectionMatrix;

layout (std140) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
};


void main()
{
	gl_Position = MVP2D * vec4(position,0.0,1.0);
    Color = color;
}
