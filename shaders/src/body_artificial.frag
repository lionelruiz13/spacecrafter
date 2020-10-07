//
// body artificial
//
#version 450
#pragma debug(on)
#pragma optimize(off)

layout (set = 1, binding=0) uniform sampler2D mapTexture;
layout(constant_id = 0) const bool useTexture = false;

layout (location=0) out vec4 FragColor;

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;
layout (location=3) in float Ambient;

//uniform bool useTexture;

layout (binding=2, set=2) uniform LightInfo {
    vec3 Position;	// Light position in eye coords.
    vec3 Intensity;	// A,D,S intensity
} Light;

layout (push_constant) uniform MaterialInfo {
    layout (offset=0) vec3 Ka;  	// Ambient reflectivity
    layout (offset=12) float Ns;	// Specular factor
    layout (offset=16) vec3 Kd;		// Diffuse reflectivity
    layout (offset=32) vec3 Ks;		// Specular reflectivity
} Material;

void phongModel( vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec ) {
    vec3 s = normalize(vec3(Light.Position) - pos);
    vec3 v = normalize(-pos.xyz);
    vec3 r = reflect( -s, norm );
    float sDotN = max( dot(s,norm), 0.0 + Ambient);
    vec3 diffuse = Light.Intensity * Material.Kd * sDotN;
    vec3 ambient = Light.Intensity * Material.Ka * sDotN;
    spec = vec3(0.0);
    if( sDotN > 0.0 )
        spec = Light.Intensity * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Ns );
    ambAndDiff = ambient + diffuse ;
}

float simple(vec3 pos, vec3 norm) {
	vec3 s = normalize(Light.Position - pos);
	float sDotN = max( dot(s,norm), 0.0 );
	return sDotN;
}

void main()
{
  vec4 texColor;
  if (useTexture)
		texColor = texture(mapTexture, TexCoord);
	else
		texColor = vec4(Material.Ka,1.0);

  vec3 ambAndDiff, spec;
  phongModel(Position, Normal, ambAndDiff, spec );
  FragColor = (vec4( ambAndDiff, 1.0 ) * texColor) + vec4(spec,1.0);
}

