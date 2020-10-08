//
// my moon tessellation
//

#version 430 core
#pragma debug(on)
#pragma optimize(off)

//layout (quads, equal_spacing) in;
layout (triangles, equal_spacing) in;
//layout (isolines, equal_spacing) in;
//layout (triangles, fractional_odd_spacing)in;

/*
layout (location=0) in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[gl_MaxPatchVertices];


layout (location=0) out gl_PerVertex {
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};
*/

layout (location=0) in vec2 TexCoordIn[];
layout (location=1) in vec3 NormalIn[];
    //~ in vec3 tangent;

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec3 Normal;
    //~ out vec3 tangent;
    //out vec3 tessCoord;


void main(void)
{
    /*
    tes_out.tessCoord = vec3(   gl_TessCoord.x,
                                gl_TessCoord.y,
                                gl_TessCoord.z);
    */
    vec2 TexCoord=TexCoordIn[0]*gl_TessCoord.x+
                       TexCoordIn[1]*gl_TessCoord.y+
                       TexCoordIn[2]*gl_TessCoord.z;
    TexCoord = TexCoord;

    Normal= NormalIn[0]*gl_TessCoord.x+
            NormalIn[1]*gl_TessCoord.y+
            NormalIn[2]*gl_TessCoord.z;

    //~ vec3 normal= tes_in[0].Normal*gl_TessCoord.x+
                 //~ tes_in[1].Normal*gl_TessCoord.y+
                 //~ tes_in[2].Normal*gl_TessCoord.z;
    //~ vec3 tangent= tes_in[0].tangent*gl_TessCoord.x+
                  //~ tes_in[1].tangent*gl_TessCoord.y+
                  //~ tes_in[2].tangent*gl_TessCoord.z;

 // first we compute the position
    vec4 position=(gl_TessCoord.x * gl_in[0].gl_Position)+
                  (gl_TessCoord.y * gl_in[1].gl_Position)+
                  (gl_TessCoord.z * gl_in[2].gl_Position);
                  position.w=1.0;

    //~ position.xyz= position.xyz/length(position.xyz);
    
    

    gl_Position =  position;
}
