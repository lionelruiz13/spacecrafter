//
// body normal tessellation
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (binding=0) uniform sampler2D mapTexture;
layout (binding=2) uniform sampler2D shadowTexture;

uniform float SunHalfAngle; 
uniform vec3 MoonPosition1; 
uniform float MoonRadius1; 

// uniform vec3 MoonPosition; 
uniform float Ambient;
uniform vec3 UmbraColor;

// smooth in vec2 TexCoord;
// in float Ambient; 
// in vec3 Position;
// in float NdotL;
// in vec3 Light; 

out vec4 FragColor;

//~ smooth in vec2 TexCoord;

in GS_OUT {
    vec3 Position;
    vec2 TexCoord;
    vec3 Normal;
    vec3 Light;
	float NdotL;
} fs_in;

void main(void)
{
	vec3 umbra = vec3(0.0, 0.0, 0.0);
	float diffuse = max(fs_in.NdotL, 0.0);
	vec4 color = texture(mapTexture, fs_in.TexCoord);
	float shadowScale = 1.0;
	if(diffuse != 0.0) {
		vec3 moon;
		float moonHalfAngle;
		vec2 ratio;
		float distance;
		vec3 lookup;
		float shadowScale = 1.0;
		if(MoonRadius1 != 0.0) {
			moon = MoonPosition1 - fs_in.Position;
			moonHalfAngle = atan( MoonRadius1/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			shadowScale = shadowScale * lookup.r; 
			diffuse = diffuse * lookup.r;
			umbra = UmbraColor;
		}
	}
	FragColor = vec4(color.rgb*(min(diffuse*(shadowScale+umbra*max(0.0,1.0-shadowScale))+Ambient, 1.0)), 1.0);
}

