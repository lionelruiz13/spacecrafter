//
// my moon tessellation
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (binding=5) uniform sampler2D mapTexture;
layout (binding=6) uniform sampler2D normalTexture;
layout (binding=7) uniform sampler2D shadowTexture;

layout (binding=1) uniform moonFrag {
	vec3 MoonPosition1;
	float MoonRadius1;
	vec3 UmbraColor;
	float SunHalfAngle;
	float Ambient;
};

// uniform vec3 MoonPosition; 
//uniform float Ambient;
//uniform vec3 UmbraColor;

// smooth in vec2 TexCoord;
// in float Ambient; 
// in vec3 Position;
// in float NdotL;
// in vec3 Light; 

layout (location=0) out vec4 FragColor;

//~ smooth in vec2 TexCoord;

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;
layout (location=3) in vec3 Light;
layout (location=4) in vec3 TangentLight;


void main(void)
{ 
	vec3 umbra = vec3(0.0, 0.0, 0.0);
	vec4 color = texture(mapTexture, TexCoord);
	vec3 light_b = normalize(TangentLight);
	vec3 normal_b = 2.0 * vec3(texture(normalTexture, TexCoord)) - vec3(1.0); 
	float diffuse = max(dot(normal_b, light_b), 0.0);
	float shadowScale = 1.0;
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
			shadowScale = shadowScale * lookup.r;
			umbra = UmbraColor;
		}
	}	
		
	FragColor = vec4(color.rgb*min(diffuse*(shadowScale+umbra*max(0.0,1.0-shadowScale))+Ambient,1.0), color.a);
} 

/*
void main(void)
{
	vec3 umbra = vec3(0.4,0.6,0.2); // UmbraColor;
	float diffuse = max(fs_in.NdotL, 0.0);
	vec4 color = vec4(1.0,1.0,1.0,1.0);
	//vec4 color = texture(mapTexture, TexCoord);
	float shadowScale = 1.0;
	if(diffuse != 0.0) {
		vec3 moon;
		float moonHalfAngle;
		vec2 ratio;
		float distance;
		vec3 lookup;
		float shadowScale = 1.0;
		if(MoonRadius1 != 0.0) {
			moon = MoonPosition1 - Position;
			moonHalfAngle = atan( MoonRadius1/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			shadowScale = shadowScale * lookup.r; 
			diffuse = diffuse * lookup.r;
			//umbra = UmbraColor;
		}
	}
	FragColor = vec4(color.rgb*min(diffuse*(shadowScale+umbra*max(0.0,1.0-shadowScale))+Ambient, 1.0),  color.a);
	//FragColor = vec4(color.rgb*min(diffuse*(shadowScale+umbra*max(0.0,1.0-shadowScale))+Ambient,1.0), color.a);
}
*/
