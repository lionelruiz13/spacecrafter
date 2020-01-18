//
// body artificial
//
#version 420
#pragma debug(on)
#pragma optimize(off)

in colorFrag
{
    vec3 Position;
    vec2 TexCoord;
    vec3 Normal;
    float Ambient;
} cfIn;

// in vec3 Position;
// in vec2 TexCoord;
// in vec3 Normal;
// in float Ambient;

uniform sampler2D Tex1;

uniform bool useTexture;

struct LightInfo {
  vec3 Position;  // Light position in eye coords.
  vec3 Intensity; // A,D,S intensity
};
uniform LightInfo Light;

struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Kd;            // Diffuse reflectivity
  vec3 Ks;            // Specular reflectivity
  float Ns;   		 // Specular factor
};
uniform MaterialInfo Material;

layout( location = 0 ) out vec4 FragColor;

void phongModel( vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec ) {
    vec3 s = normalize(vec3(Light.Position) - pos);
    vec3 v = normalize(-pos.xyz);
    vec3 r = reflect( -s, norm );
    float sDotN = max( dot(s,norm), 0.0 + cfIn.Ambient);
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

//~ subroutine( UsedTextureType )
void useTexturedMaterial(out vec4 texColor)
{
	texColor = texture(Tex1, cfIn.TexCoord);
}

//~ subroutine( UsedTextureType )
void useColoredMaterial(out vec4 texColor)
{
	texColor = vec4(Material.Ka,1.0);
}


void main() {
    vec4 texColor;
    if (useTexture)
		useTexturedMaterial(texColor);
	else
		useColoredMaterial(texColor);
    vec3 ambAndDiff, spec;
    phongModel( cfIn.Position, cfIn.Normal, ambAndDiff, spec );
    FragColor = (vec4( ambAndDiff, 1.0 ) * texColor) + vec4(spec,1.0);

	//~ FragColor = texColor * simple(Position, Normal);
    // FragColor = vec4(1.0,0.0,0.0,1.0);
}

// in vec3 Position;
// in vec2 TexCoord;
// in vec3 Normal;
// in float Ambient;