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
struct meshFrag {
	// Sun-frame projection rows, receiver-folded (ShadowProjection.hpp):
	// shadowPos = (dot(row0.xyz, PositionEye) + row0.w, ... row1 ...).
	Vec4f shadowRow0;
	Vec4f shadowRow1;
	int nbShadowingBodies;
	int _pad[3];
	struct ShadowingBody {
		Vec4f posRadius;      // xy = caster center in sun-frame (rel. receiver), z = disc radius, w unused
		Vec4f absorbtionIdx;  // rgb = caster shadowAbsorbtion, w = layer index (float for sampler2DArray)
	} shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

#endif /* end of include guard: BODY_SHADER_INTERFACE_HPP_ */
