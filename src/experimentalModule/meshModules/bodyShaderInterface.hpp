#ifndef BODY_SHADER_INTERFACE_HPP_
#define BODY_SHADER_INTERFACE_HPP_

#include "tools/vecmath.hpp"

// ============================================================================
// UBO layouts of the shared body_* shaders (shaders/body_normal.* etc.).
// These describe the SHADER interface - they belong to whichever path draws
// with those shaders. Single authority (I2): moved here from bodyShader.hpp
// (2026-07-12, S1(b) re-home); bodyShader.hpp includes this header for the
// old path - an old->new include edge that dies with the old path
// (INTENT.md 12 retirement map). Field ORDER and TYPES are the GPU-visible
// layout - never reorder without changing the shaders.
// ============================================================================

struct globalVertProj {
	Mat4f ModelViewMatrix;
	Mat4f NormalMatrix;
	Vec3f clipping_fov;
	float planetRadius;
	Vec3f LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

// OLD-PATH-ONLY since S5: the analytic eclipse-map occluder feed (Gen-1,
// shadow-paths.md A1) - the new path draws with bodyMesh.frag/meshFrag below
// (Gen-2 projected shadows replace the LUT outright [vixy: 2026-07-12]).
// Dies with the old path (INTENT.md 12 retirement map).
struct globalFrag {
	Vec3f MoonPosition1;
	float MoonRadius1;
	Vec3f MoonPosition2;
	float MoonRadius2;
	Vec3f MoonPosition3;
	float MoonRadius3;
	Vec3f MoonPosition4;
	float MoonRadius4;
	float SunHalfAngle;
};

// Receiver-side cap on simultaneous casters = the shader UBO array size
// (bodyMesh.frag MAX_SHADOW_CASTERS). Single authority for both sides: the
// orchestration truncates its occlusion-ordered list here, so - unlike the
// old shadowingBodies[4]-vs-max_shadow_cast-8 silent mismatch - overflow
// drops the LEAST significant shadows, deterministically.
#define MAX_SHADOW_CASTERS_PER_RECEIVER 8

// std140 mirror of bodyMesh.frag binding 1 (new path, S5). Field order and
// types are the GPU-visible layout - never reorder without the shader.
// Also binding 1 of the row-2 mid families (bodyTes*.frag, bodyLayered*.frag)
// - the S5 receive block is shared verbatim across every disc fragment.
struct meshFrag {
	// Sun-frame projection rows, receiver-folded (ShadowProjection.hpp):
	// shadowPos = (dot(row0.xyz, PositionEye) + row0.w, ... row1 ...).
	Vec4f shadowRow0;
	Vec4f shadowRow1;
	int nbShadowingBodies;
	int _pad[3];
	struct ShadowingBody {
		Vec4f posRadius;      // xy = caster center in sun-frame (rel. receiver), z = disc radius, w unused
		Vec4f absorbtionIdx;  // rgb = entry shadowAbsorbtion, w = layer index (float for sampler2DArray)
		Vec4f clip;           // eye-space half-space gate: apply iff dot(P, xyz) + w <= 0
		                      // ((0,0,0,-1) = always; planar casters - ShadowProjection.hpp)
	} shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

// std140 mirror of binding 2 of the MESH_TES family (bodyTes shaders; old
// globalTescGeom, body_tes.vert/tesc + *.tese). TesParam = [min_tes_lvl,
// max_tes_lvl, altimetry_level]; the tese displaces by
// 0.01*TesParam[2]*heightmap. Values come from the shared BodyTesselation
// (both-paths seam - LayeredMesh fills per frame, Scalable transitions
// identical by construction).
struct meshTescGeom {
	Vec3i TesParam;
	int _pad;
};

// std140 mirror of binding 0 of the MESH_RAYMARCH family
// (body_tes_shadow.vert, REUSED VERBATIM - old ShadowVert,
// bodyShader.hpp:232-241). ModelViewMatrix = m * scale(1,1,oneMinusOblateness)
// where m = bodyMat * zrot(axisRotation) - i.e. the near-component matrix
// with computeBodyToSurface()'s +PI/2 removed: the ray-march reconstructs
// texture longitude from atan(y,x) instead of mesh texcoords (old CoI
// convention, body_bigbody.cpp drawCenterOfInterest).
struct rayMarchVert {
	Mat4f ModelViewMatrix;
	float WorldToModelMatrix[12]; // std140 mat3 (3 vec4-padded columns); fill via Mat4f::setMat3 = transpose of m's linear part
	float zNear;                  // renderer.getClippingFov()[0]
	float zRange;                 // [1] - [0]
	float fov;                    // [2]
	float radius;                 // finalRadius = min(r*(1+altimetryFactor), distance - r/64)
};

// std140 mirror of binding 1 of the MESH_RAYMARCH family (bodyRayMarch*.frag;
// old ShadowFrag with the mat3 ShadowMatrix REPLACED by the two S5 sun-frame
// rows, pre-folded through the model matrix so shadowPos lands in eye-space
// AU and caster posRadius entries are consumed unchanged (no initialRadius
// normalization - it existed because the old frames mixed unit systems,
// shadow-paths.md E "Units").
struct rayMarchFrag {
	// rowL.xyz = finalRadius * (row.xyz . linearColumns(MV)),
	// rowL.w = row.w + dot(row.xyz, MV.translation)
	// so dot(rowL.xyz, samplePosUnit) + rowL.w == S5 shadowPos of the
	// eye-space ground point (height term neglected, old-parity class).
	Vec4f shadowRow0;
	Vec4f shadowRow1;
	Vec3f lightDirection;        // body-local, sun -> body: normalize(m^T * (bodyPos - lightPos))
	float sinSunAngle;           // 2*starRadius/|bodyPos - lightPos|, guard max(x, 1e-6)
	float heightMapDepthLevel;   // altimetryCoef = radius/finalRadius
	float heightMapDepth;        // altimetryFactor * altimetryCoef
	float squaredHeightMapDepthLevel;
	float sunDeviation;          // sin(atmosphere_sun_deviation deg)
	Vec3f atmColor;              // atmosphere_ambient_r/g/b
	float atmDeviation;          // sin(atmosphere_ambient_deviation deg)
	int nbShadowingBodies;
	int _pad[3];
	meshFrag::ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

#endif /* end of include guard: BODY_SHADER_INTERFACE_HPP_ */
