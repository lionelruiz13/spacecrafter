//
//	body_halo
//
#version 420
#pragma debug(on)
#pragma optimize(off)

//externe
layout(binding=0, set=1) uniform sampler2D mapTexture;
layout(push_constant) uniform UniformBufferObject {
    vec3 Color;
    layout (offset=12) float cmag;
} ubo;

//in
layout(location=0) in vec2 TexCoord;

//out
layout(location=0) out vec3 FragColor;

void main(void)
{
	vec3 tex_color = ubo.cmag * vec3(texture(mapTexture,TexCoord)).rgb;
	FragColor = vec3(tex_color.r * ubo.Color.r, tex_color.g * ubo.Color.g, tex_color.b * ubo.Color.b);
	//~ FragColor = vec3(1.0, 0.0, 0.0);
	
	//Coloration en rouge, on indique au programme que l'on veut du rouge !
	// couleur R G B A ou A est la transparence
	//~ FragColor = vec4 (1.0, 0.0, 0.0, 1.0);
}

