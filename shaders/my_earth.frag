//
// body night tessellation
//

#version 430
#pragma debug(on)
#pragma optimize(off)
#pragma optionNV(fastprecision off)

layout (binding=0) uniform sampler2D mapTexture;   //DayTexture
layout (binding=2) uniform sampler2D NightTexture;
layout (binding=3) uniform sampler2D SpecularTexture;
layout (binding=1) uniform sampler2D shadowTexture;

uniform int Clouds;
layout (binding=4) uniform sampler2D CloudTexture;
layout (binding=5) uniform sampler2D CloudNormalTexture;
uniform float SunHalfAngle;

uniform vec3 MoonPosition1;
uniform float MoonRadius1;
uniform vec3 MoonPosition2;
uniform float MoonRadius2;
uniform vec3 MoonPosition3;
uniform float MoonRadius3;
uniform vec3 MoonPosition4;
uniform float MoonRadius4;

//~ smooth in vec2 TexCoord;

out vec4 FragColor;

in GS_OUT{
    vec3 Position;
    vec2 TexCoord;
    vec3 Normal;
    vec3 TangentLight;
    vec3 Light;
    vec3 ViewDirection;
}fs_in;


//~ in vec3 Normal;
//~ in vec3 Position;
//~ in vec3 TangentLight;
//~ in vec3 Light;
//~ in vec3 ViewDirection;


void main(void)
{
	vec3 daytime = vec3(texture(mapTexture, fs_in.TexCoord));
	vec3 night = vec3(texture(NightTexture, fs_in.TexCoord));
	vec3 reflective = vec3(texture(SpecularTexture, fs_in.TexCoord)); 
	vec3 halfangle = normalize(fs_in.Light + fs_in.ViewDirection);
	const float specularExp = 25.0;
	float NdotH = dot(fs_in.Normal, halfangle);
	vec3 specular = reflective * vec3(pow(max(0.0, NdotH), specularExp)); 
	float NdotL = dot(fs_in.Normal, fs_in.Light);
	float diffuse = max(0.0, NdotL);
	vec3 daycolor = (diffuse + specular) * daytime;
//	float nightBrightness = 1.0 * night.x + 1.0 * night.y+ 1.0 *night.z; 
//	vec3 nightcolor = step( 0.0, nightBrightness ) * night;
	vec3 nightcolor = night;
	vec4 cloudColor = vec4(0.0, 0.0, 0.0, 0.0);
	float cloudDiffuse = 0.0;
	if(Clouds != 0) {
		cloudColor = texture(CloudTexture, fs_in.TexCoord);
		vec3 light_b = normalize(fs_in.TangentLight);
		vec3 normal_b = 2.0 * vec3(texture(CloudNormalTexture, fs_in.TexCoord)) - vec3(1.0); 
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
			moon = MoonPosition1 - fs_in.Position;
			moonHalfAngle = atan( MoonRadius1/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			shadowScale = shadowScale * lookup.r;
		}
		if(MoonRadius2 != 0.0) {
			moon = MoonPosition2 - fs_in.Position;
			moonHalfAngle = atan( MoonRadius2/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			shadowScale = shadowScale * lookup.r;
		}
		if(MoonRadius3 != 0.0) {
			moon = MoonPosition3 - fs_in.Position;
			moonHalfAngle = atan( MoonRadius3/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)); 
			shadowScale = shadowScale * lookup.r;
		}
		if(MoonRadius4 != 0.0) {
			moon = MoonPosition4 - fs_in.Position;
			moonHalfAngle = atan( MoonRadius4/ length(moon) ); 
			distance = acos(dot(fs_in.Light, normalize(moon)));
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
	//~ FragColor = vec4(1.0, 0.0, 0.0, 1.0); 
	
}
