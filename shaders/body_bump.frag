//
// body bump
//
#version 420
#pragma debug(on)
#pragma optimize(off)

layout (binding=0) uniform sampler2D mapTexture;
layout (binding=1) uniform sampler2D normalTexture;
layout (binding=2) uniform sampler2D shadowTexture;
uniform float SunHalfAngle;

uniform vec3 UmbraColor;
uniform vec3 MoonPosition1; 
uniform float MoonRadius1; 
uniform vec3 MoonPosition2; 
uniform float MoonRadius2; 
uniform vec3 MoonPosition3; 
uniform float MoonRadius3; 
uniform vec3 MoonPosition4; 
uniform float MoonRadius4;

smooth in vec2 TexCoord;
in float Ambient; 

out vec4 FragColor;
in vec3 Normal;
in vec3 Position;
in vec3 TangentLight;
in vec3 Light;

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
		if(MoonRadius2 != 0.0) { 
			moon = MoonPosition2 - Position;
			moonHalfAngle = atan( MoonRadius2/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon))); 
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)).rgb; 
			diffuse = diffuse * lookup.r; 
		} 
		if(MoonRadius3 != 0.0) { 
			moon = MoonPosition3 - Position;
			moonHalfAngle = atan( MoonRadius3/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon))); 
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)).rbg; 
			diffuse = diffuse * lookup.r; 
		} 
		if(MoonRadius4 != 0.0) { 
			moon = MoonPosition4 - Position;
			moonHalfAngle = atan( MoonRadius4/ length(moon) ); 
			distance = acos(dot(Light, normalize(moon))); 
			ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			lookup = vec3(texture(shadowTexture, ratio)).rgb; 
			diffuse = diffuse * lookup.r; 
		} 
	}
	//~ FragColor = vec4(color.rgb*diffuse*(shadowScale+umbra*max(0.0,1.0-shadowScale)), color.a);
	FragColor = vec4(color.rgb*min(diffuse*(shadowScale+umbra*max(0.0,1.0-shadowScale))+Ambient,1.0), color.a);
	//~ FragColor= vec4(1.0,1.0,0.0,1.0);
} 




//~ uniform sampler2D mapTexture;
//~ uniform sampler2D normalTexture;
//~ uniform sampler2D shadowTexture;
//~ uniform float SunHalfAngle;
//~ uniform vec3 UmbraColor;
//~ uniform vec3 MoonPosition1;
//~ uniform float MoonRadius1;
//~ uniform vec3 MoonPosition2;
//~ uniform float MoonRadius2;
//~ uniform vec3 MoonPosition3;
//~ uniform float MoonRadius3;
//~ uniform vec3 MoonPosition4;
//~ uniform float MoonRadius4;
//~ varying vec2 TexCoord;
//~ varying vec3 TangentLight;
//~ varying vec3 Normal;
//~ varying vec3 Light;
//~ varying vec3 Position;

//~ void main(void)
//~ {
	//~ vec3 umbra = vec3(0.0, 0.0, 0.0);
	//~ vec4 color = texture(mapTexture, TexCoord);
	//~ vec3 light_b = normalize(TangentLight);
	//~ vec3 normal_b = 2.0 * vec3(texture(normalTexture, TexCoord)) - vec3(1.0); 
	//~ float diffuse = max(dot(normal_b, light_b), 0.0);
	//~ float shadowScale = 1.0;
	//~ if(diffuse != 0.0) {
		//~ vec3 moon;
		//~ float moonHalfAngle;
		//~ vec2 ratio;
		//~ float distance;
		//~ vec3 lookup;
		//~ if(MoonRadius1 != 0.0) {
			//~ moon = MoonPosition1 - Position;
			//~ moonHalfAngle = atan( MoonRadius1/ length(moon) ); 
			//~ distance = acos(dot(Light, normalize(moon)));
			//~ ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			//~ ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			//~ lookup = vec3(texture(shadowTexture, ratio)); 
			//~ shadowScale = shadowScale * lookup.r;
			//~ umbra = UmbraColor;
		//~ }
		//~ if(MoonRadius2 != 0.0) {
			//~ moon = MoonPosition2 - Position;
			//~ moonHalfAngle = atan( MoonRadius2/ length(moon) ); 
			//~ distance = acos(dot(Light, normalize(moon)));
			//~ ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			//~ ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			//~ lookup = vec3(texture(shadowTexture, ratio)); 
			//~ shadowScale = shadowScale * lookup.r;
		//~ }
		//~ if(MoonRadius3 != 0.0) {
			//~ moon = MoonPosition3 - Position;
			//~ moonHalfAngle = atan( MoonRadius3/ length(moon) ); 
			//~ distance = acos(dot(Light, normalize(moon)));
			//~ ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			//~ ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			//~ lookup = vec3(texture(shadowTexture, ratio)); 
			//~ shadowScale = shadowScale * lookup.r;
		//~ }
		//~ if(MoonRadius4 != 0.0) {
			//~ moon = MoonPosition4 - Position;
			//~ moonHalfAngle = atan( MoonRadius4/ length(moon) ); 
			//~ distance = acos(dot(Light, normalize(moon)));
			//~ ratio.y = clamp(moonHalfAngle/SunHalfAngle/51.2, 0.0, 1.0); 
			//~ ratio.x = distance/(moonHalfAngle + SunHalfAngle); 
			//~ lookup = vec3(texture(shadowTexture, ratio)); 
			//~ shadowScale = shadowScale * lookup.r;
		//~ }
	//~ }
	//~ gl_FragColor = vec4(color.rgb*diffuse*(shadowScale+umbra*max(0.0,1.0-shadowScale)), color.a); 
//~ }
