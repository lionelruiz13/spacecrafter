//
// ringplanet
//
#version 420
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

uniform sampler2D Texture;
smooth in vec2 TexCoord;
in float PlanetHalfAngle;
in float Separation;
in float SeparationAngle;
in float NdotL;

out vec4 Color;

void main(void)
{
	vec4 color = vec4(texture(Texture, TexCoord));

	float diffuse = clamp(max(NdotL, -NdotL*0.2), 0.0, 1.0);
	if(SeparationAngle < PlanetHalfAngle) diffuse = 0.0;
	float reflected = 0.3 * max(-1.0*Separation, 0.0);
	Color = vec4(color.rgb*(diffuse+reflected+0.00), color.a); 
}
