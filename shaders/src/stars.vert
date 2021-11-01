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

void main(void)
{
	mag = Mag;
	color = Color;
	gl_Position = vec4(Position, 0.0,1.0);
}
