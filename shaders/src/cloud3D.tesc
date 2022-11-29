//
// cloud 3D
//
#version 420

layout(vertices=3) out;

#define ID gl_InvocationID

layout (location=0) in vec3 position[];
layout (location=1) in vec3 texCoord[];
layout (location=2) in vec4 color[];
layout (location=3) in int visible[];
layout (location=4) in mat4 invModel[];
layout (location=8) in float lodFactor[];

layout (location=0) out vec3 positionOut[];
layout (location=1) out vec3 texCoordOut[];
layout (location=2) patch out vec4 colorOut;
layout (location=3) patch out mat4 invModelOut;
layout (location=7) patch out float lodFactorOut;

layout (binding=1) uniform fov {
    vec3 clipping_fov;
};

void main()
{
    if (ID == 0) {
        colorOut = color[0];
        invModelOut = invModel[0];
        lodFactorOut = lodFactor[0];
        vec3 tes = vec3(distance(gl_in[1].gl_Position.xy, gl_in[2].gl_Position.xy),
                        distance(gl_in[0].gl_Position.xy, gl_in[2].gl_Position.xy),
                        distance(gl_in[0].gl_Position.xy, gl_in[1].gl_Position.xy));

        float Visible = float(visible[0] & visible[1] & visible[2] | int(clipping_fov.z > 2.5));
        tes = clamp(tes * 32, 1., 64.) * Visible;

        gl_TessLevelOuter[0] = tes[0];
        gl_TessLevelOuter[1] = tes[1];
        gl_TessLevelOuter[2] = tes[2];

        gl_TessLevelInner[0]=(tes[0] + tes[1] + tes[2])/3;
    }
    positionOut[ID] = position[ID];
    texCoordOut[ID] = texCoord[ID];
}
