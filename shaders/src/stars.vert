//
//	STARS
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec2 Position;
layout (location = 1) in vec3 Color;
layout (location = 2) in float Mag;

layout (location=0) out float mag;
layout (location=1) out vec3 color;
layout (location=2) out vec2 posOut;

layout (set = 0, binding = 1) uniform cam_block
{
	ivec4 viewport;
	ivec4 viewport_center;
	vec4 main_clipping_fov;
	mat4 MVP2D;
	float ambient;
	float time;
	bool allsphere;
};

void main(void)
{
	mag = Mag;
	color = Color;
	posOut = Position;
}
