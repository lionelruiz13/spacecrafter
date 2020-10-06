// skygrid

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=1, set=1) uniform ubo {
	vec3 color;
	float fader;
};

layout (location=0) in float intensityColor;

layout (location=0) out vec4 Color;

void main(void)
{
	Color = vec4 (color*intensityColor, fader);
}
