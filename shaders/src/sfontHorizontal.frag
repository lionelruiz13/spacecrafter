//
//	sfontHorizontal
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=1) uniform sampler2D tex;
layout(push_constant) uniform pushConstants {
	vec4 Color;
};

layout (location=0) in vec2 TexCoord;
layout (location=1) in vec2 TexCoord2;
 
layout (location=0) out vec4 FragColor;

void main(void)
{
	// vec4 textureColor = vec4(texture(tex,TexCoord2)).rgba;
	// vec4 mapColor = vec4(texture(tex,TexCoord)).rgba;
	// textureColor.r = mapColor.r * Color.r;
	// textureColor.g = mapColor.g * Color.g;
	// textureColor.b = mapColor.b * Color.b;
	// FragColor = textureColor;

	// Commented code do the same as the following :
	FragColor = vec4(vec3(texture(tex, TexCoord)), texture(tex, TexCoord2).a) * Color;
}
