#ifndef BODY_SHADER_INTERFACE_HPP_
#define BODY_SHADER_INTERFACE_HPP_

#include "tools/vecmath.hpp"

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

#define MAX_SHADOW_CASTERS_PER_RECEIVER 8

struct meshFrag {
	int nbShadowingBodies;
	int _pad[3];
	struct ShadowingBody {
		Vec4f posRadius;      // xy = caster center in sun-frame (rel. receiver), z = disc radius, w unused
		Vec4f absorbtionIdx;  // rgb = TRANSMISSION absorption aT (solid 1; rings = material), w = layer index
		Vec4f clip;           // eye-space half-space gate: apply iff dot(P, xyz) + w <= 0
		Vec4f row0, row1;
		Vec4f glow;           // rgb = refraction glow gR (umbra-only chroma,
		                      // physical-sharp composition [vixy: 2026-07-18]
		                      // - receivedShadows.glsl derivation), w unused
	} shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

struct meshTescGeom {
	Vec3i TesParam;
	int _pad;
};

struct rayMarchVert {
	Mat4f ModelViewMatrix;
	float WorldToModelMatrix[12]; // std140 mat3 (3 vec4-padded columns); fill via Mat4f::setMat3 = transpose of m's linear part
	float zNear;                  // renderer.getClippingFov()[0]
	float zRange;                 // [1] - [0]
	float fov;                    // [2]
	float radius;                 // finalRadius = min(r*(1+altimetryFactor), distance - r/64)
};

struct rayMarchFrag {
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

// std140 mirror of bodyRing.vert binding 0. Field order/types = GPU layout.
struct bodyRingVert {
	Mat4f ModelViewMatrix;         // near-list matrix (spin-folded; ring is axisymmetric)
	Mat4f ModelViewMatrixInverse;
	Vec3f clipping_fov;
	float RingScale;               // body scaling factor mc (old rings->multiplyRadius analog)
	Vec3f PlanetPosition;          // eye-space body center (planet-shine input)
	float SunnySideUp;             // observer above/below ring plane (h test, ring.cpp:252)
	Vec3f LightDirection;          // eye-space, body -> sun, normalized (old computeDraw:1034)
	float fadingFactor;            // asteroid cross-fade; 100000 until row 5 (old else-branch)
};

// std140 mirror of bodyRing.frag binding 1 - the receive block (disc-receiver
// idiom: entries unfolded, eye-space P).
struct bodyRingFrag {
	int nbShadowingBodies;
	int _pad[3];
	meshFrag::ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

struct ojmVert {
	float NormalMatrix[12]; // std140 mat3
};

// std140 mirror of body_artificial.vert/geom binding 1 set 2 (artGeom).
struct ojmGeom {
	Mat4f ModelViewMatrix;  // eye model matrix * scaling(radius) (old drawBody)
	Vec3f clipping_fov;
	float _pad;
};

// std140 mirror of body_artificial_tex/notex.frag binding 2 set 2 (LightInfo)
// - the PLAIN rows' light block (old uLight; eye-space light position).
struct ojmLight {
	Vec3f Position;  // light position in eye coords (old eye_sun)
	float _p0;
	Vec3f Intensity; // A,D,S intensity (old hardcode {1,1,1})
	float _p1;
};

struct ojmShadowBlock {
	float ShadowMatrix[12]; // std140 mat3: model -> sun-frame NDC
	float ModelMatrix[12];  // std140 mat3: model -> eye (rotation+scale)
	Vec3f ModelPosition;    // eye-space body center
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
