//
// landscape2T
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;
//~ layout (location=2)in vec3 normal; // useless unitl now

out ValueTex
{
	vec2 TexCoord;
} valueTex;

void main()
{
	gl_Position = vec4(position,1.0);
    valueTex.TexCoord = texcoord;
}
