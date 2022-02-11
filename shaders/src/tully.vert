// tully

#version 420
#pragma debug(on)
#pragma optimize(off)

layout(constant_id = 0) const bool whiteColor = false;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec3 inColor;
layout (location = 2) in float inTexture;
layout (location = 3) in float inScale;

//uniform mat4 Mat;

layout (location=0) out float scale;
layout (location=1) out float texture;
layout (location=2) out vec3 color;


void main(void)
{
	gl_Position = inPosition;

	color = whiteColor ? vec3(1.0) : inColor;
	scale= inScale;
	texture = inTexture;
}
