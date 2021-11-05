//
//	VIEWPORT SHAPE
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout(constant_id=0) const int radius = 512;
layout(constant_id=1) const int centerX = 512;
layout(constant_id=2) const int centerY = 512;
//~ smooth in vec2 TexCoord;

layout(location=0) out vec4 FragColor;

void main(void)
{
	float opacity = min(0.5 + length(ivec2(gl_FragCoord.x - centerX, gl_FragCoord.y - centerY)) - radius, 1.0);

	if (opacity < 0.)
		discard;
	FragColor = vec4(0.0,0.0,0.0,opacity);
}
