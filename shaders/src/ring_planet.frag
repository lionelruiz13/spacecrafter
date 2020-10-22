//
// ringplanet
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (binding=1) uniform sampler2D Texture;
layout (location=0) in vec2 TexCoord;
layout (location=1) in float PlanetHalfAngle;
layout (location=2) in float Separation;
layout (location=3) in float SeparationAngle;
layout (location=4) in float NdotL;

layout (location=0) out vec4 Color;

void main(void)
{
	vec4 color = vec4(texture(Texture, TexCoord));

	float diffuse = (SeparationAngle < PlanetHalfAngle) ? 0 : clamp(max(NdotL, -NdotL*0.2), 0.0, 1.0);
	float reflected = 0.3 * max(-Separation, 0.0);
	Color = vec4(color.rgb*(diffuse+reflected), color.a); 
}
