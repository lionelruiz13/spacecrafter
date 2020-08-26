//
//	object_base pointer
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (location=0) in vec2 position;
layout (location=4) in float _pos;

out int posC;


void main()
{
	posC = int(_pos);
	gl_Position = vec4(position,0.0,1.0);
}

