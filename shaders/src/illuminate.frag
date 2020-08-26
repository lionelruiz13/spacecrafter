//
//	illuminate
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0) uniform sampler2D mapTexture;

in DataFrag
{
	vec2 texCoord;
	vec3 texColor;
} dataFrag;

out vec4 FragColor;

void main(void)
{
	vec4 textureColor = texture(mapTexture,dataFrag.texCoord).rgba;
	textureColor.r *= dataFrag.texColor.r;
	textureColor.g *= dataFrag.texColor.b;
	textureColor.b *= dataFrag.texColor.g;
	FragColor = textureColor;
	
	//~ FragColor = vec4(1.0,0.0,0.0,1.0);
}
