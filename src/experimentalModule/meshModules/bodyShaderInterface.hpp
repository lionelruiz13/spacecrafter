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

#endif /* end of include guard: BODY_SHADER_INTERFACE_HPP_ */
