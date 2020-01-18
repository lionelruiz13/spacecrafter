//
//	VIEWPORT SHAPE
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec2 position;
//~ layout (location=1)in vec2 texcoord;

//externe
//~ uniform sampler2D texunit0;

//out
//~ smooth out vec2 TexCoord;

void main()
{
	gl_Position = vec4(position,0.,1.);
    //~ TexCoord = texcoord;
}
