#version 430 core
#pragma debug(on)
#pragma optimize(off)

//layout (quads, equal_spacing) in;
layout (triangles, equal_spacing) in;
//layout (isolines, equal_spacing) in;
//layout (triangles, fractional_odd_spacing)in;

//uniform sampler2D texture;
uniform sampler2D heightMap;
//uniform sampler2D normalMap;

/*
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
*/

uniform mat4 model;
uniform mat4 vp;

/*
in vec3 gl_TessCoord;
//in int gl_PatchVerticesIn;
in int gl_PrimitiveID;
patch in float gl_TessLevelOuter[4];
patch in float gl_TessLevelInner[2];*/
in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[gl_MaxPatchVertices];

out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

// we'll change this with a uniform later.
/*const*/ vec3 lightPosition=vec3(0.0,0.0,0.0);

in TCS_OUT{
    in vec2 uv;
    in vec3 normal;
    in vec3 tangent;
    //in mat3 TBN;
}tes_in[];

out TES_OUT{
    out vec2 uv;
    out vec3 normal;
    out vec3 tangent;
    out vec3 lightRay;
    out vec3 tessCoord;
    //out mat3 TBN;
}tes_out;


void main(void)
{
    tes_out.tessCoord = vec3(   gl_TessCoord.x,
                                gl_TessCoord.y,
                                gl_TessCoord.z);

    //tes_out.param=(tes_in[0].param+tes_in[1].param+tes_in[2].param)*0.33;
    vec2 uvCoordinates=tes_in[0].uv*gl_TessCoord.x+
                       tes_in[1].uv*gl_TessCoord.y+
                       tes_in[2].uv*gl_TessCoord.z;
    tes_out.uv=  uvCoordinates;

    tes_out.normal= tes_in[0].normal*gl_TessCoord.x+
                    tes_in[1].normal*gl_TessCoord.y+
                    tes_in[2].normal*gl_TessCoord.z;
    /*tes_out.tangent= tes_in[0].tangent*gl_TessCoord.x+
                     tes_in[1].tangent*gl_TessCoord.y+
                     tes_in[2].tangent*gl_TessCoord.z;*/

    // retrieving normal and tangent
    vec3 normal= tes_in[0].normal*gl_TessCoord.x+
                 tes_in[1].normal*gl_TessCoord.y+
                 tes_in[2].normal*gl_TessCoord.z;
    vec3 tangent= tes_in[0].tangent*gl_TessCoord.x+
                  tes_in[1].tangent*gl_TessCoord.y+
                  tes_in[2].tangent*gl_TessCoord.z;


    // computing bitangent and also TBN matrix
    vec3 bitangent=cross(normal,tangent);
    //vec3 bitangent=cross(tangent,normal);
    mat3 TBN=mat3(tangent,bitangent,normal);




    /*for(int i=0;i<9;i++)
        tes_out.TBN[i]= tes_in[0].TBN[i]*gl_TessCoord.x+
                        tes_in[1].TBN[i]*gl_TessCoord.y+
                        tes_in[2].TBN[i]*gl_TessCoord.z;*/


 // first we compute the position
    vec4 position=(gl_TessCoord.x * gl_in[0].gl_Position)+
                  (gl_TessCoord.y * gl_in[1].gl_Position)+
                  (gl_TessCoord.z * gl_in[2].gl_Position);
                  position.w=1.0;



    // then we use the heightmap to get the r
    // himalaya height: 8800 m.
    // earth ray      : 6356752 m.
    //position.xyz= position.xyz/length(position.xyz) *(1.0+texture2D(heightMap,uvCoordinates).x *(8800.0/(6356752.0+8800.0)));
    position.xyz= position.xyz/length(position.xyz) *(1.0+texture(heightMap,uvCoordinates).x *0.055);


    // get the space coordinates for light calculations
    position=model*position;

    /*tes_out.lightRay= vec3(1.0,1.0,
                                dot(
                                        (mat3(model)*normal).xy,
                                        normalize(lightPosition.xy-position.xy )
                                    )
                           );*/

    TBN = (mat3(model))*TBN;
    //TBN = (mat3(UBOData.model))*transpose(TBN);

    //TBN = mat3(inverse(transpose(model)))*TBN;

    tes_out.lightRay= transpose(TBN)* normalize(lightPosition-position.xyz);
    //tes_out.lightRay= TBN* normalize(lightPosition-position.xyz);

    //tes_out.lightRay= normalize(transpose(TBN)* normalize(lightPosition-position.xyz));



    gl_Position =  vp * position;
}









































