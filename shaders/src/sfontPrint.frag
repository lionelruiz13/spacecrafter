//
//	sfontPrint
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=1) uniform sampler2D mapTexture;
layout(push_constant) uniform pushConstants {
	vec4 Color;
};

layout (location=0) in vec2 TexCoord;
 
layout (location=0) out vec4 FragColor;

void main(void)
{
	vec4 textureColor = vec4(texture(mapTexture,TexCoord)).rgba;
	textureColor.r *= Color.r;
	textureColor.g *= Color.g;
	textureColor.b *= Color.b;
	textureColor.a *= Color.a;
	FragColor = textureColor;
	//~ FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}


