//
//	screen_fader
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec2 position;

void main()
{
	gl_Position = vec4(position,0.,1.);
}
