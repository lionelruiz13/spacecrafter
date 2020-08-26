//
//	STARS
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec2 Position;
layout (location = 4) in float Mag;
layout (location = 3) in vec3 Color;

out vertexData
{
	float mag;
	vec3 color;
} vertexOut;
 
void main(void)
{
	vertexOut.mag = Mag;
	vertexOut.color = Color;
	gl_Position = vec4(Position, 0.0,1.0);
}
