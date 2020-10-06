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
	float dist_squared = dot(pos, pos);

	if (dist_squared > radius*radius)
		FragColor = vec4(0.0,0.0,0.0,1.0);
	else
		discard;
}
