//
//	body_trace
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)


layout (location = 0) in vec3 Position;

void main(void)
{
	gl_Position = vec4(Position,1.0);
}
