//
//	illuminate
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0) uniform sampler2D mapTexture;

layout (location=0) in vec2 texCoord;
layout (location=1) in vec3 texColor;

layout (location=0) out vec4 FragColor;

void main(void)
{
	vec4 textureColor = texture(mapTexture,texCoord).rgba;
	textureColor.r *= texColor.r;
	textureColor.g *= texColor.b;
	textureColor.b *= texColor.g;
	FragColor = textureColor;
	
	//~ FragColor = vec4(1.0,0.0,0.0,1.0);
}
