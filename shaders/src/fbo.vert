//
// fbo
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;

//out
smooth out vec2 TexCoord;


void main()
{
	gl_Position = vec4(position, 1.0);
    TexCoord = texcoord;
}
