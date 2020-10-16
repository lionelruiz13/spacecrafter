//
//	VIEWPORT SHAPE
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout(binding=0) uniform uRadius {int radius;};
layout(binding=1) uniform uDecalage {
	int decalagex;
	int decalagey;
};
//~ smooth in vec2 TexCoord;

layout(location=0) out vec4 FragColor;

void main(void)
{
	vec2 pos = gl_FragCoord.xy-vec2(radius+decalagex,radius+decalagey);
	float opacity = min(0.5 + length(pos) - radius, 1.0);

	if (opacity < 0.)
		discard;
	FragColor = vec4(0.0,0.0,0.0,opacity);
}
