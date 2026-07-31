//
// bodyStarSurface - vertex stage of the near-surface star family's photosphere
// (B12; design note claude/b12-design.md).
//
// body_normal.vert MINUS the incident-light varyings (a self-lit surface has no
// NdotL and no ambient term) PLUS the eye-space NORMAL, which is what the
// limb-darkening law needs. The varying set is exact on purpose: a superset
// would fire WARNING-Shader-OutputNotConsumed against the zero-validation bar
// (the same reason the tessellated rows carry one .tese each).
//
// The UBO block is globalVertProj VERBATIM (bodyShaderInterface.hpp) - one CPU
// authority for every sphere-mesh vertex block. planetRadius and LightPosition
// are declared (the block layout is shared) and unread here.
//
// Position is built from the SCALED geometry, not from planetRadius as
// body_normal.vert does: this varying's only consumer is the view-angle cosine
// of the DRAWN surface, so it must be the drawn point. (body_normal.vert's
// unscaled form exists for its shadow/light frame, which this family has not.)
//
#version 420

layout (binding=0) uniform globalVertProj {
	mat4 ModelViewMatrix;
	mat4 NormalMatrix;
	vec3 clipping_fov;
	float planetRadius;
	vec3 LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

#include <custom_project.glsl>

layout (location=0) in vec3 position;
layout (location=1) in vec2 texcoord;
layout (location=2) in vec3 normal;

layout (location=0) out vec2 TexCoord;
layout (location=1) out vec3 Position;
layout (location=2) out vec3 Normal;

void main()
{
	vec3 Position0;
	Position0.x = position.x * planetScaledRadius;
	Position0.y = position.y * planetScaledRadius;
	Position0.z = position.z * planetScaledRadius * planetOneMinusOblateness;
	gl_Position = custom_project(Position0, ModelViewMatrix, clipping_fov);

	Position = vec3(ModelViewMatrix * vec4(Position0, 1.0));
	Normal = normalize(mat3(NormalMatrix) * normal);
	TexCoord = texcoord;
}
