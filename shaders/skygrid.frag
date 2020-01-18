// skygrid

#version 420
#pragma debug(on)
#pragma optimize(off)

uniform vec3 color;
uniform float fader;

in colorFrag
{
	float intensityColor;
} cfIn;

out vec4 Color;

void main(void)
{
	Color = vec4 (color*cfIn.intensityColor, fader);
}
