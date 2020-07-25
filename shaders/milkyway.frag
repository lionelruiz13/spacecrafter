//milkyway

#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D texunit0;
layout (binding=1) uniform sampler2D texunit1;

uniform float cmag;
uniform float texTransit;

subroutine vec3 useTexType();
subroutine uniform useTexType useTex;

vec3 tex_color;
smooth in vec2 TexCoord;
 
out vec4 FragColor;
 
subroutine(useTexType)
vec3 useOneTex()
{
	vec3 tex_color0 = vec3(texture(texunit0,TexCoord)).rgb;
	tex_color0 *= cmag;
	return tex_color0;
}

subroutine(useTexType)
vec3 useTwoTex()
{
	vec3 tex_color0 = vec3(texture(texunit0,TexCoord)).rgb;
	vec3 tex_color1 = vec3(texture(texunit1,TexCoord)).rgb;
	vec3 tex_color = mix (tex_color0, tex_color1, texTransit);

	tex_color *= cmag;
	return tex_color;
}


void main(void)
{
	FragColor = vec4(useTex(), 1.f);
}
