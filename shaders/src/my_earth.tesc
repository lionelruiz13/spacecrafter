//
// my earth tessellation
//

#version 430 core
#pragma debug(on)
#pragma optimize(off)

layout(vertices=3) out;

//NEW UNIFORMS
layout (binding=3) uniform globalTescGeom {
	uniform ivec3 TesParam;         // [min_tes_lvl, max_tes_lvl, coeff_altimetry]
};

layout (binding=4) uniform globalTesc {
	uniform mat4 ViewProjection;
	uniform mat4 Model;
};

layout(location=0) in vec3 glPositionIn[];
layout(location=1) in vec2 TexCoordIn[];
layout(location=2) in vec3 NormalIn[];

layout(location=0) out vec3 glPositionOut[];
layout(location=1) out vec2 TexCoordOut[];
layout(location=2) out vec3 NormalOut[];
    //~ out vec3 tangent;

#define ID gl_InvocationID

int MinTessLevel=TesParam[0];
int MaxTessLevel=TesParam[1];
const float MaxDepth=8.0;
const float MinDepth=1.0;

void main(void)
{
    if(ID==0)
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

        gl_TessLevelInner[0]=1; // I think it should be tessLevel
        gl_TessLevelInner[1]=1; // Not needed

        gl_TessLevelOuter[0]=1; // I think it should be MaxTessLevel
        gl_TessLevelOuter[1]=1; // I think it should be MaxTessLevel
        gl_TessLevelOuter[2]=1; // I think it should be MaxTessLevel
        gl_TessLevelOuter[3]=1; // Not needed
    }

    TexCoordOut[ID] = TexCoordIn[ID];
    NormalOut[ID] = NormalIn[ID];
    glPositionOut[ID] = glPositionIn[ID];
    //~ tcs_out[ID].tangent = tcs_in[ID].tangent;
    // gl_out[ID].gl_Position = gl_in[ID].gl_Position;
}
