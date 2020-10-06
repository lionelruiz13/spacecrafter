//
// OJM frag with no SUN
//
#version 420
#pragma debug(on)
#pragma optimize(off)


//~ subroutine void UsedTextureType(vec4 texColor );
//~ subroutine uniform UsedTextureType UsedTexture;

layout (location=0) in vec3 Position;
layout (location=1) in vec2 TexCoord;
layout (location=2) in vec3 Normal;

layout (set = 1, binding=0) uniform sampler2D Tex1;

//uniform bool useTexture;

layout (push_constant) uniform MaterialInfo {
    layout (offset=0) vec3 Ka;  	// Ambient reflectivity
    layout (offset=12) vec3 Kd;		// Diffuse reflectivity
    layout (offset=24) vec3 Ks;		// Specular reflectivity
    layout (offset=36) float Ns;	// Specular factor
} Material;

layout (push_constant) uniform customPush {
    layout (offset=40) float T;
};

layout (push_constant) uniform LightInfo {
    layout (offset=44) vec4 Position;  // Light position in eye coords.
    layout (offset=60) vec3 Intensity; // A,D,S intensity
} Light;

layout( location = 0 ) out vec4 FragColor;

void phongModel( vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec ) {
    vec3 s = normalize(vec3(Light.Position) - pos);
    vec3 v = normalize(-pos.xyz);
    vec3 r = reflect( -s, norm );
    vec3 ambient = Light.Intensity * Material.Ka;
    float sDotN = max( dot(s,norm), 0.0 );
    vec3 diffuse = Light.Intensity * Material.Kd * sDotN;
    spec = vec3(0.0);
    if( sDotN > 0.0 )
        spec = Light.Intensity * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Ns );
    ambAndDiff = ambient + diffuse;
}


void mainTextured()
{
    FragColor = texture(Tex1, TexCoord);
    //~ vec3 ambAndDiff, spec;
    //~ vec4 texColor = mix(texture( Tex1, TexCoord ), vec4(Material.Ka,1.0), useTexture);
    //~ phongModel( Position, Normal, ambAndDiff, spec );
    //~ FragColor = (vec4( ambAndDiff, 1.0 ) * texColor) + vec4(spec,1.0);
    //~ FragColor = vec4(0.0,0.0,1.0,1.0);
}

void mainTextureless()
{
    FragColor = vec4(Material.Ka,T);
    //~ vec3 ambAndDiff, spec;
    //~ vec4 texColor = mix(texture( Tex1, TexCoord ), vec4(Material.Ka,1.0), useTexture);
    //~ phongModel( Position, Normal, ambAndDiff, spec );
    //~ FragColor = (vec4( ambAndDiff, 1.0 ) * texColor) + vec4(spec,1.0);
    //~ FragColor = vec4(0.0,0.0,1.0,1.0);
}
