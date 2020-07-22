//
// landscape2T
//

#version 420
#pragma debug(on)
#pragma optimize(off)

//layout
//~ layout (location=0)in vec3 position;
//~ layout (location=1)in vec2 texcoord;
//~ layout (location=2)in vec3 normal; // useless unitl now

//layout
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

//externe
uniform sampler2D texunit0;
uniform sampler2D texunit1;
uniform float sky_brightness;
uniform float fader;
//uniform mat4 ModelViewProjectionMatrix; // unused now

//out
//~ smooth out vec2 TexCoord;


in ValueTex
{
	vec2 TexCoord;
} valueTex[];

out ValueTexFrag
{
	vec2 TexCoord;
} valueTexFrag;

#include <fisheye.glsl>

//////////////////// PROJECTION FISHEYE ////////////////////////////////


void main(void)
{
	vec4 pos1, pos2, pos3;

	pos1 = custom_project(gl_in[0].gl_Position);
	pos2 = custom_project(gl_in[1].gl_Position);
	pos3 = custom_project(gl_in[2].gl_Position);

	if ( pos1.w==1.0 && pos2.w==1.0 && pos3.w==1.0) {

		gl_Position = custom_unproject(pos1);
		valueTexFrag.TexCoord = valueTex[0].TexCoord;
		//~ faderColor.indice = valueFader[0].indice;
		EmitVertex();

		gl_Position = custom_unproject(pos2);
		valueTexFrag.TexCoord = valueTex[1].TexCoord;
		//~ faderColor.indice = valueFader[1].indice;
		EmitVertex();

		gl_Position = custom_unproject(pos3);
		valueTexFrag.TexCoord = valueTex[2].TexCoord;
		//~ faderColor.indice = valueFader[1].indice;
		EmitVertex();

		EndPrimitive();	
	}
}
