#version 420

layout (location=0) in vec3 Color;
layout (location=0) out vec4 FragColor;
layout (location=1) in float PlanetHalfAngle;
layout (location=2) in float Separation;
layout (location=3) in float SeparationAngle;
layout (location=4) in float NdotL;

void main(void)
{
	float diffuse = (SeparationAngle < PlanetHalfAngle) ? 0 : clamp(max(NdotL, -NdotL*0.2), 0.0, 1.0);
	float reflected = 0.3 * max(-Separation, 0.0);
	FragColor = vec4(Color*(diffuse+reflected), 1.0);
}
