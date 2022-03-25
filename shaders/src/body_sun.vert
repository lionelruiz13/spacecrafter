//
// body sun
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (binding=0) uniform uModelViewMatrix {mat4 ModelViewMatrix;};
#include <cam_block.glsl>
#include <fisheye.glsl>

//layout
layout (location=0)in vec3 position;
layout (location=1)in vec2 texcoord;


//externe
layout(binding=3) uniform ubo1 {float planetScaledRadius;};
layout(binding=2) uniform ubo2 {vec3 clipping_fov;};

//out
layout(location=0) out vec2 TexCoord;

void main()
{
	//~ gl_Position = ModelViewProjectionMatrix * vec4(position, 1.0);
	vec3 Position =position * planetScaledRadius;
	gl_Position = fisheyeProject(Position, clipping_fov);
    TexCoord = texcoord;
}
