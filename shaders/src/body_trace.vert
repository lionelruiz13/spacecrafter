//
//	body_trace
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)


layout (location = 0) in vec4 Position;

void main(void)
{
	gl_Position = Position;
}
