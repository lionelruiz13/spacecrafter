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

//uniform bool useTexture;

layout (push_constant) uniform pushConstant {
    layout (offset=0) vec3 Ka;  	// Ambient reflectivity
    layout (offset=12) float Ns;	// Specular factor
    layout (offset=16) vec3 Kd;		// Diffuse reflectivity
    layout (offset=32) vec3 Ks;		// Specular reflectivity

    layout (offset=28) float T;

    layout (offset=48) vec4 LightPosition;  // Light position in eye coords.
    layout (offset=64) vec3 LightIntensity; // A,D,S intensity
};

layout( location = 0 ) out vec4 FragColor;

void phongModel( vec3 pos, vec3 norm, out vec3 ambAndDiff, out vec3 spec ) {
    vec3 s = normalize(vec3(LightPosition) - pos);
    vec3 v = normalize(-pos.xyz);
    vec3 r = reflect( -s, norm );
    vec3 ambient = LightIntensity * Ka;
    float sDotN = max( dot(s,norm), 0.0 );
    vec3 diffuse = LightIntensity * Kd * sDotN;
    spec = vec3(0.0);
    if( sDotN > 0.0 )
        spec = LightIntensity * Ks * pow( max( dot(r,v), 0.0 ), Ns );
    ambAndDiff = ambient + diffuse;
}


void main()
{
    FragColor = vec4(Ka,T);
    //~ vec3 ambAndDiff, spec;
    //~ vec4 texColor = mix(texture( Tex1, TexCoord ), vec4(Ka,1.0), useTexture);
    //~ phongModel( Position, Normal, ambAndDiff, spec );
    //~ FragColor = (vec4( ambAndDiff, 1.0 ) * texColor) + vec4(spec,1.0);
    //~ FragColor = vec4(0.0,0.0,1.0,1.0);
}
