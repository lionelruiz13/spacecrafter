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
layout (location=2) in float Ambient; 
// in vec3 Position;
// in float NdotL;
// in vec3 Light; 

layout (location=0) out vec4 FragColor;

//~ smooth in vec2 TexCoord;

layout (location=0) in GS_OUT {
    vec3 Position;
    vec2 TexCoord;
    vec3 Normal;
    vec3 Light;
    float NdotL;
} fs_in;

void main(void)
{ 
	float diffuse = max(fs_in.NdotL, 0.0);
	vec3 color = vec3(texture(mapTexture, fs_in.TexCoord)).rgb;

	if(diffuse != 0.0) {
		vec3 moon;
		float moonHalfAngle;
		vec2 ratio;
		float distance;
		vec3 lookup;
		if(MoonRadius1 != 0.0) {
			moon = MoonPosition1 - fs_in.Position;
			moonHalfAngle = atan( MoonRadius1/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			diffuse = diffuse * lookup.r;
		}
		if(MoonRadius2 != 0.0) {
			moon = MoonPosition2 - fs_in.Position;
			moonHalfAngle = atan( MoonRadius2/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			diffuse = diffuse * lookup.r;
		}
		if(MoonRadius3 != 0.0) {
			moon = MoonPosition3 - fs_in.Position;
			moonHalfAngle = atan( MoonRadius3/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			diffuse = diffuse * lookup.r;
		}
		if(MoonRadius4 != 0.0) {
			moon = MoonPosition4 - fs_in.Position;
			moonHalfAngle = atan( MoonRadius4/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			diffuse = diffuse * lookup.r;
		}
	}
	FragColor = vec4(color*(min(diffuse+Ambient, 1.0)), 1.0);
}

