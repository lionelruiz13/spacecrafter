//
// starNav
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D texunit0;

//~ in vec3 Color;
out vec4 FragColor;


in Interpolators
{
	vec2 TexCoord;
	vec3 TexColor;
} interData;



void main(void)
{
	vec4 textureColor = texture(texunit0, interData.TexCoord);

	if (textureColor.a == 0.)
	discard;

	FragColor = vec4 (interData.TexColor.r * textureColor.r, interData.TexColor.g * textureColor.g, interData.TexColor.b * textureColor.b, 1.0);
}

