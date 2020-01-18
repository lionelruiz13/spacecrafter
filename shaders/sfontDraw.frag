//
//	sfontDraw
//
#version 420
#pragma debug(on)
#pragma optimize(off)


layout (binding=0) uniform sampler2D mapTexture;
uniform float fader;

smooth in vec2 TexCoord;
 
out vec4 FragColor;

void main(void)
{
	vec4 textureColor = vec4(texture(mapTexture,TexCoord)).rgba;
	textureColor.a *= fader;
	FragColor = textureColor;
	//~ FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}


