//
// body normal tessellation
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (binding=6) uniform sampler2D mapTexture;
layout (binding=7) uniform sampler2D shadowTexture;

layout (binding=1) uniform globalFrag {
	vec3 MoonPosition1;
	float MoonRadius1;
	vec3 MoonPosition2;
	float MoonRadius2;
	vec3 MoonPosition3;
	float MoonRadius3;
	vec3 MoonPosition4;
	float MoonRadius4;
	float SunHalfAngle;
};

// uniform vec3 MoonPosition; 
//uniform float Ambient;

// smooth in vec2 TexCoord;
// in vec3 Position;
// in float NdotL;
// in vec3 Light; 

layout (location=0) out vec4 FragColor;

//~ smooth in vec2 TexCoord;

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;
layout (location=3) in vec3 Light;
layout (location=4) in float NdotL;
layout (location=5) in float Ambient; 

void main(void)
{ 
	float diffuse = max(NdotL, 0.0);
	vec3 color = vec3(texture(mapTexture, TexCoord)).rgb;

	if(diffuse != 0.0) {
		vec3 moon;
		float moonHalfAngle;
		vec2 ratio;
		float distance;
		vec3 lookup;
		if(MoonRadius1 != 0.0) {
			moon = MoonPosition1 - Position;
			moonHalfAngle = atan( MoonRadius1/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			diffuse = diffuse * lookup.r;
		}
		if(MoonRadius2 != 0.0) {
			moon = MoonPosition2 - Position;
			moonHalfAngle = atan( MoonRadius2/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			diffuse = diffuse * lookup.r;
		}
		if(MoonRadius3 != 0.0) {
			moon = MoonPosition3 - Position;
			moonHalfAngle = atan( MoonRadius3/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			diffuse = diffuse * lookup.r;
		}
		if(MoonRadius4 != 0.0) {
			moon = MoonPosition4 - Position;
			moonHalfAngle = atan( MoonRadius4/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			diffuse = diffuse * lookup.r;
		}
	}
	FragColor = vec4(color*(min(diffuse+Ambient, 1.0)), 1.0);
}

