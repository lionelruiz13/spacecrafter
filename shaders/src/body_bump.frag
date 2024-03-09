//
// body bump
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=3) uniform sampler2D mapTexture;
layout (binding=4) uniform sampler2D normalTexture;
layout (binding=5) uniform sampler2D shadowTexture;

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

layout (binding=2) uniform custom {
	vec3 UmbraColor;
};

layout (location=0) in vec2 TexCoord;
layout (location=1) in float Ambient;

layout (location=0) out vec4 FragColor;
layout (location=2) in vec3 Normal;
layout (location=3) in vec3 Position;
layout (location=4) in vec3 TangentLight;
layout (location=5) in vec3 Light;

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
		float lookup;
		if(MoonRadius1 != 0.0) {
			moon = MoonPosition1 - Position;
			moonHalfAngle = atan(MoonRadius1/length(moon));
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0);
			ratio.x = distance/(moonHalfAngle + SunHalfAngle);
			shadowScale = texture(shadowTexture, ratio).r;
			umbra = UmbraColor;
		}
		if(MoonRadius2 != 0.0) {
			moon = MoonPosition2 - Position;
			moonHalfAngle = atan(MoonRadius2/length(moon));
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0);
			ratio.x = distance/(moonHalfAngle + SunHalfAngle);
			lookup = texture(shadowTexture, ratio).r;
			diffuse = diffuse * lookup;
		}
		if(MoonRadius3 != 0.0) {
			moon = MoonPosition3 - Position;
			moonHalfAngle = atan(MoonRadius3/length(moon));
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0);
			ratio.x = distance/(moonHalfAngle + SunHalfAngle);
			lookup = texture(shadowTexture, ratio).r;
			diffuse = diffuse * lookup;
		}
		if(MoonRadius4 != 0.0) {
			moon = MoonPosition4 - Position;
			moonHalfAngle = atan(MoonRadius4/length(moon));
			distance = acos(dot(Light, normalize(moon)));
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0);
			ratio.x = distance/(moonHalfAngle + SunHalfAngle);
			lookup = texture(shadowTexture, ratio).r;
			diffuse = diffuse * lookup;
		}
	}
	FragColor = vec4(color.rgb*min(diffuse*(shadowScale+umbra*max(0.0,1.0-shadowScale))+Ambient,1.0), color.a);
}
