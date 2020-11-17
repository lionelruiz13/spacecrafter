#version 420

layout (location=0) out vec4 FragColor;
layout (location=1) in float PlanetHalfAngle;
layout (location=2) in float Separation;
layout (location=3) in float SeparationAngle;
layout (location=4) in float NdotL;
layout (location=5) in vec3 Color;

void main(void)
{
	float diffuse = 0;
	if (SeparationAngle >= PlanetHalfAngle)
		diffuse = clamp(max(NdotL, -NdotL*0.2), 0.0, 1.0);
	float reflected = 0.3 * max(-Separation, 0.0);
	float intensity = diffuse + reflected;
	FragColor = vec4(Color.r*intensity, Color.g*intensity, Color.b*intensity, 1.0);
}
