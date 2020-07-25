//
// my earth tessellation
//

#version 430 core
#pragma debug(on)
#pragma optimize(off)

//layout (quads, equal_spacing) in;
layout (triangles, equal_spacing) in;
//layout (isolines, equal_spacing) in;
//layout (triangles, fractional_odd_spacing)in;


uniform mat4 model;
uniform mat4 vp;


// in gl_PerVertex
// {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// } gl_in[gl_MaxPatchVertices];


// out gl_PerVertex {
//   vec4 gl_Position;
//   float gl_PointSize;
//   float gl_ClipDistance[];
// };



in TCS_OUT{
    in vec4 glPosition;
    in vec2 TexCoord;
    in vec3 Normal;
    //~ in vec3 tangent;
}tes_in[];

out TES_OUT{
    out vec4 glPosition; 
    out vec2 TexCoord;
    out vec3 Normal;
    //~ out vec3 tangent;
    out vec3 tessCoord;
}tes_out;


void main(void)
{
    tes_out.tessCoord = vec3(   gl_TessCoord.x,
                                gl_TessCoord.y,
                                gl_TessCoord.z);

    vec2 TexCoord=tes_in[0].TexCoord*gl_TessCoord.x+
                       tes_in[1].TexCoord*gl_TessCoord.y+
                       tes_in[2].TexCoord*gl_TessCoord.z;
    tes_out.TexCoord=  TexCoord;

    tes_out.Normal= tes_in[0].Normal*gl_TessCoord.x+
                    tes_in[1].Normal*gl_TessCoord.y+
                    tes_in[2].Normal*gl_TessCoord.z;

    //~ vec3 normal= tes_in[0].Normal*gl_TessCoord.x+
                 //~ tes_in[1].Normal*gl_TessCoord.y+
                 //~ tes_in[2].Normal*gl_TessCoord.z;
    //~ vec3 tangent= tes_in[0].tangent*gl_TessCoord.x+
                  //~ tes_in[1].tangent*gl_TessCoord.y+
                  //~ tes_in[2].tangent*gl_TessCoord.z;

 // first we compute the position
    vec4 position=(gl_TessCoord.x * tes_in[0].glPosition)+
                  (gl_TessCoord.y * tes_in[1].glPosition)+
                  (gl_TessCoord.z * tes_in[2].glPosition);
                  position.w=1.0;

    //~ position.xyz= position.xyz/length(position.xyz);
    
    
    tes_out.glPosition =  position;
    // gl_Position =  position;
}









































