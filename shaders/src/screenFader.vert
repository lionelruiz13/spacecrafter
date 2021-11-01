//
//	screen_fader
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0) in vec4 position;

void main()
{
	gl_Position = position;
}
