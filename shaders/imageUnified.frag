//
//	imageUnified
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;

uniform float fader;
uniform bool transparency;
uniform vec4 noColor;

smooth in vec2 TexCoord;
 
out vec4 FragColor;

void main(void)
{
	vec4 tex_color = texture(mapTexture,TexCoord);
	tex_color.a *= fader;

	if (transparency) {
		vec3 diffVec = abs(tex_color.rgb-noColor.rgb);
		float delta = noColor.a;
		if ((diffVec.r<delta) && (diffVec.g<delta) && (diffVec.b<delta))
			discard;
	}

	FragColor = tex_color;
}
