//
// my moon tessellation
//

#version 430 core
#pragma debug(on)
#pragma optimize(off)

layout(vertices=3) out;

//NEW UNIFORMS
uniform mat4 ViewProjection;
uniform mat4 Model;
uniform ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]

in gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_in[gl_MaxPatchVertices];


out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
} gl_out[];


in VS_OUT{
    in vec2 TexCoord;
    in vec3 Normal;
    //~ in vec3 tangent;
}tcs_in[];

out TCS_OUT{
    out vec2 TexCoord;
    out vec3 Normal;
    //~ out vec3 tangent;
}tcs_out[];

#define ID gl_InvocationID

int MinTessLevel=TesParam[0];
int MaxTessLevel=TesParam[1];
const float MaxDepth=8.0;
const float MinDepth=1.0;

void main(void)
{
    // Position in camera coordinates
    vec4 p = ViewProjection*Model[3].xyzw;

    // 0.0=close 1.0=far
    float ratio=(abs(p.z-1.0)-MinDepth)/(MaxDepth-MinDepth);


    float depth = clamp (ratio,0.0,1.0);
    depth=1.0-depth;
    depth=depth*depth*depth*depth*depth*depth*depth*depth;

    depth=1.0-depth;

    float tessLevel= mix(MaxTessLevel,MinTessLevel,depth);

    if(ID==0)
    {
        gl_TessLevelInner[0]=float(tessLevel);

        gl_TessLevelOuter[0]=float(tessLevel);
        gl_TessLevelOuter[1]=float(tessLevel);
        gl_TessLevelOuter[2]=float(tessLevel);
    }


    tcs_out[ID].TexCoord      = tcs_in[ID].TexCoord;
    tcs_out[ID].Normal  = tcs_in[ID].Normal;
    //~ tcs_out[ID].tangent = tcs_in[ID].tangent;

    gl_out[ID].gl_Position = gl_in[ID].gl_Position;
}
