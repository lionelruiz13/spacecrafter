// skygrid

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location = 0) in vec3 Position;
layout (location = 1) in float IntensityColor;


out ColorVertex
{
	float intensityColor;
} cvOut;

void main(void)
{
	gl_Position = vec4(Position,1.0);
	cvOut.intensityColor = IntensityColor;
}
