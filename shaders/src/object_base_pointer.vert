//
//	object_base pointer
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location=0) in vec4 position; // vec2
layout (location=1) in float _pos;

layout (location=0) out int posC;


void main()
{
	posC = int(_pos);
	gl_Position = position;
}

