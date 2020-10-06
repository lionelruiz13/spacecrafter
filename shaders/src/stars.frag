//
//	STARS
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D texunit0;

layout (location=0) in vec2 TexCoord;
layout (location=1) in vec3 TexColor;
 
layout (location=0) out vec4 FBColor;

void main(void)
{
	vec4 textureColor = texture(texunit0, TexCoord);

	if (textureColor.a == 0.)
		discard;

	FBColor = vec4 (TexColor.r * textureColor.r, TexColor.g * textureColor.g, TexColor.b * textureColor.b, 1.0);
}
