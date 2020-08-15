//
// body sun
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

#include <fisheye.glsl>

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;

#include <cam_block.glsl>

//externe
uniform float planetScaledRadius;
uniform vec3 clipping_fov;

//out
smooth out vec2 TexCoord;

void main()
{
	//~ gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
	vec3 Position =position * planetScaledRadius;
	gl_Position = fisheyeProject(Position, clipping_fov);
    TexCoord = texcoord;
}
