//
// my earth tessellation
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (binding=5) uniform sampler2D mapTexture;   //DayTexture
layout (binding=6) uniform sampler2D shadowTexture;
layout (binding=7) uniform sampler2D NightTexture;
layout (binding=8) uniform sampler2D SpecularTexture;

layout(constant_id = 0) const bool Clouds = false;
layout (binding=9) uniform sampler2D CloudTexture;
layout (binding=10) uniform sampler2D CloudNormalTexture;

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

layout (location=0) out vec4 FragColor;

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;
layout (location=3) in vec3 TangentLight;
layout (location=4) in vec3 Light;
layout (location=5) in vec3 ViewDirection;

void main(void)
{
	vec3 daytime = vec3(texture(mapTexture, TexCoord));
	vec3 night = vec3(texture(NightTexture, TexCoord));
	vec3 reflective = vec3(texture(SpecularTexture, TexCoord)); 
	vec3 halfangle = normalize(Light + ViewDirection);
	const float specularExp = 25.0;
	float NdotH = dot(Normal, halfangle);
	vec3 specular = reflective * vec3(pow(max(0.0, NdotH), specularExp)); 
	float NdotL = dot(Normal, Light);
	float diffuse = max(0.0, NdotL);
	vec3 daycolor = (diffuse + specular) * daytime;
	vec3 nightcolor = night;
	vec4 cloudColor = vec4(0.0, 0.0, 0.0, 0.0);
	float cloudDiffuse = 0.0;
	if(Clouds) {
		cloudColor = texture(CloudTexture, TexCoord);
		vec3 light_b = normalize(TangentLight);
		vec3 normal_b = 2.0 * vec3(texture(CloudNormalTexture, TexCoord)) - vec3(1.0); 
		cloudDiffuse = max(dot(normal_b, light_b), 0.0);
	}
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
		}
		if(MoonRadius2 != 0.0) {
			moon = MoonPosition2 - Position;
			moonHalfAngle = atan( MoonRadius2/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			shadowScale = shadowScale * lookup.r;
		}
		if(MoonRadius3 != 0.0) {
			moon = MoonPosition3 - Position;
			moonHalfAngle = atan( MoonRadius3/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			shadowScale = shadowScale * lookup.r;
		}
		if(MoonRadius4 != 0.0) {
			moon = MoonPosition4 - Position;
			moonHalfAngle = atan( MoonRadius4/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			shadowScale = shadowScale * lookup.r;
		}
	}
	vec3 color = daycolor*shadowScale; 
	if (diffuse <= 0.1) {
		nightcolor = nightcolor * smoothstep(0.0, 0.1, 0.1 - diffuse); 
		color = max(color, nightcolor); 
	}
	FragColor = vec4(mix(color, cloudColor.rgb*cloudDiffuse*shadowScale, cloudColor.a), 1.0); 
}
