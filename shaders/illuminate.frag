//
//	illuminate
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0) uniform sampler2D mapTexture;

smooth in vec2 TexCoord;
smooth in vec3 TexColor;

out vec4 FragColor;

void main(void)
{
	vec4 textureColor = texture(mapTexture,TexCoord).rgba;
	textureColor.r *= TexColor.r;
	textureColor.g *= TexColor.b;
	textureColor.b *= TexColor.g;
	FragColor = textureColor;
	
	//~ FragColor = vec4(1.0,0.0,0.0,1.0);
}
