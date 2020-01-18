//
//	illuminate
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0) uniform sampler2D mapTexture;
uniform vec3 Color;

smooth in vec2 TexCoord;
 
out vec4 FragColor;

void main(void)
{
	vec4 textureColor = texture(mapTexture,TexCoord).rgba;
	textureColor.r *= Color.r;
	textureColor.g *= Color.b;
	textureColor.b *= Color.g;
	FragColor = vec4(textureColor);
	
	//~ FragColor = vec4(1.0,0.0,0.0,1.0);
}
