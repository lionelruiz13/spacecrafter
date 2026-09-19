#ifndef BODY_SHADER_INTERFACE_HPP_
#define BODY_SHADER_INTERFACE_HPP_

#include "tools/vecmath.hpp"

// std140 layouts, never reorder a field without the shaders

struct globalVertProj {
	Mat4f ModelViewMatrix;
	Mat4f NormalMatrix;
	Vec3f clipping_fov;
	float planetRadius;
	Vec3f LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

// For the legacy body shaders only
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

// Must match MAX_SHADOW_CASTERS of the shaders
#define MAX_SHADOW_CASTERS_PER_RECEIVER 8

// Must match binding 1 of bodyMesh.frag, bodyTes*.frag and bodyLayered*.frag
struct meshFrag {
	int nbShadowingBodies;
	int _pad[3];
	struct ShadowingBody {
		Vec4f posRadius;      // xy = caster center in sun-frame, z = disc radius
		Vec4f absorbtionIdx;  // rgb = absorption, w = layer index
		Vec4f clip;           // eye-space, apply if dot(P, xyz) + w <= 0
		Vec4f row0, row1;     // sun-frame projection rows
		Vec4f glow;
	} shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

// Must match binding 2 of the MESH_TES family
struct meshTescGeom {
	Vec3i TesParam; // [min_tes_lvl, max_tes_lvl, altimetry_level]
	int _pad;
};

// Must match binding 0 of MESH_RAYMARCH and bodyRayMarch.frag
struct rayMarchVert {
	Mat4f ModelViewMatrix;        // without the +PI/2 of computeBodyToSurface()
	float WorldToModelMatrix[12]; // std140 mat3, fill via Mat4f::setMat3
	float zNear;
	float zRange;
	float fov;
	float radius;                 // finalRadius, altimetry headroom included
};

// Must match binding 1 of MESH_RAYMARCH, fill with fillFoldedShadows
struct rayMarchFrag {
	Vec3f lightDirection;        // body-local, sun -> body
	float sinSunAngle;
	float heightMapDepthLevel;   // altimetryCoef = radius/finalRadius
	float heightMapDepth;        // altimetryFactor * altimetryCoef
	float squaredHeightMapDepthLevel;
	float sunDeviation;
	Vec3f atmColor;
	float atmDeviation;
	int nbShadowingBodies;
	int _pad[3];
	meshFrag::ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

// Must match binding 0 of bodyRing.vert
struct bodyRingVert {
	Mat4f ModelViewMatrix;
	Mat4f ModelViewMatrixInverse;
	Vec3f clipping_fov;
	float RingScale;
	Vec3f PlanetPosition;          // eye-space
	float SunnySideUp;             // observer above/below ring plane
	Vec3f LightDirection;          // eye-space, body -> sun, normalized
	float fadingFactor;            // 100000 while there is no asteroid variant
};

// Must match binding 1 of bodyRing.frag
struct bodyRingFrag {
	int nbShadowingBodies;
	int _pad[3];
	meshFrag::ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

// Must match binding 0 set 2 of body_artificial.vert
struct ojmVert {
	float NormalMatrix[12]; // std140 mat3, fill via Mat4f::setMat3
};

// Must match binding 1 set 2 of body_artificial.vert/geom
struct ojmGeom {
	Mat4f ModelViewMatrix;  // eye model matrix * scaling(radius)
	Vec3f clipping_fov;
	float _pad;
};

// Must match binding 2 set 2 of body_artificial_tex/notex.frag
struct ojmLight {
	Vec3f Position;  // in eye coords
	float _p0;
	Vec3f Intensity; // A,D,S
	float _p1;
};

// Must match binding 2 set 2 of ojmShadowTex/Notex.frag
struct ojmShadowBlock {
	float ShadowMatrix[12]; // model -> sun-frame NDC, same as the SELF_DEPTH pass
	float ModelMatrix[12];  // mat3 model -> eye
	Vec3f ModelPosition;    // eye-space
	float _p0;
	Vec3f lightDirection;   // eye-space, direction light travels
	float _p1;
	Vec3f LightIntensity;
	float selfShadowOn;     // 1 = self-shadow depth valid this frame
	int nbShadowingBodies;
	int _pad[3];
	meshFrag::ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

#endif /* end of include guard: BODY_SHADER_INTERFACE_HPP_ */
