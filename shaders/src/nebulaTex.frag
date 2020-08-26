// nebulaTex

#version 420
#pragma debug(on)
#pragma optimize(off)

uniform float fader;
layout (binding=0) uniform sampler2D mapTexture;


out vec4 FragColor;


in FInterpolators
{
	vec2 texCoord;
} dataFrag;


void main(void)
{
	vec4 tex_color = vec4(texture(mapTexture,dataFrag.texCoord)).rgba;
	tex_color.a *= fader;
	//~ if (fader>1.)
		//~ FragColor = vec4(1.0, 0.0,0.0,1.0);
	//~ else
		FragColor = tex_color;
}
