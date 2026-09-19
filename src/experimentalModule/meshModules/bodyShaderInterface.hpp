#ifndef BODY_SHADER_INTERFACE_HPP_
#define BODY_SHADER_INTERFACE_HPP_

#include "tools/vecmath.hpp"

// std140 UBO layouts of the body shaders: field ORDER and TYPES are GPU-visible, never reorder without the shaders

struct globalVertProj {
	Mat4f ModelViewMatrix;
	Mat4f NormalMatrix;
	Vec3f clipping_fov;
	float planetRadius;
	Vec3f LightPosition;
	float planetScaledRadius;
	float planetOneMinusOblateness;
};

// Occluder feed of the legacy body shaders only; the modular path draws with meshFrag
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

// = the shaders' MAX_SHADOW_CASTERS array size; the orchestration truncates its occlusion-ordered list to it
#define MAX_SHADOW_CASTERS_PER_RECEIVER 8

// Binding 1 of bodyMesh.frag and of the mid families (bodyTes*.frag, bodyLayered*.frag): the shadow receive block
struct meshFrag {
	int nbShadowingBodies;
	int _pad[3];
	struct ShadowingBody {
		Vec4f posRadius;      // xy = caster center in sun-frame (rel. receiver), z = disc radius, w unused
		Vec4f absorbtionIdx;  // rgb = TRANSMISSION absorption aT (solid 1; rings = material), w = layer index
		Vec4f clip;           // eye-space half-space gate: apply iff dot(P, xyz) + w <= 0
		                      // ((0,0,0,-1) = always)
		// Sun-frame projection rows of THIS entry, receiver-folded:
		// shadowPos = (dot(row0.xyz, P) + row0.w, dot(row1.xyz, P) + row1.w)
		Vec4f row0, row1;
		Vec4f glow;           // rgb = refraction glow gR (umbra-only chroma), w unused
	} shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

// Binding 2 of the MESH_TES family. TesParam = [min_tes_lvl, max_tes_lvl, altimetry_level];
// the tese displaces by 0.01*TesParam[2]*heightmap
struct meshTescGeom {
	Vec3i TesParam;
	int _pad;
};

// Binding 0 of the MESH_RAYMARCH family (VERTEX|FRAGMENT): bodyRayMarch.frag re-declares it, add any field there too
// ModelViewMatrix = bodyMat * zrot(axisRotation) * scale(1,1,oneMinusOblateness), i.e. without the +PI/2 of
// computeBodyToSurface(): the ray-march reconstructs the texture longitude from atan(y,x)
struct rayMarchVert {
	Mat4f ModelViewMatrix;
	float WorldToModelMatrix[12]; // std140 mat3 (3 vec4-padded columns); fill via Mat4f::setMat3 = transpose of m's linear part
	float zNear;                  // renderer.getClippingFov()[0]
	float zRange;                 // [1] - [0]
	float fov;                    // [2]
	float radius;                 // finalRadius = min(r*(1+altimetryFactor), distance - r/64)
};

// Binding 1 of the MESH_RAYMARCH family; entries folded by fillFoldedShadows so that shadowPos lands in eye-space AU
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
	float RingScale;               // body scaling factor mc
	Vec3f PlanetPosition;          // eye-space body center (planet-shine input)
	float SunnySideUp;             // observer above/below ring plane
	Vec3f LightDirection;          // eye-space, body -> sun, normalized
	float fadingFactor;            // asteroid cross-fade; 100000 while there is no asteroid variant
};

// std140 mirror of bodyRing.frag binding 1 - the receive block (disc-receiver
// idiom: entries unfolded, eye-space P).
struct bodyRingFrag {
	int nbShadowingBodies;
	int _pad[3];
	meshFrag::ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS_PER_RECEIVER];
};

// std140 mirror of body_artificial.vert binding 0 set 2; fill via Mat4f::setMat3 of the eye-space model matrix
struct ojmVert {
	float NormalMatrix[12]; // std140 mat3
};

// std140 mirror of body_artificial.vert/geom binding 1 set 2 (artGeom).
struct ojmGeom {
	Mat4f ModelViewMatrix;  // eye model matrix * scaling(radius)
	Vec3f clipping_fov;
	float _pad;
};

// std140 mirror of body_artificial_tex/notex.frag binding 2 set 2 (LightInfo): the PLAIN rows' light block
struct ojmLight {
	Vec3f Position;  // light position in eye coords
	float _p0;
	Vec3f Intensity; // A,D,S intensity
	float _p1;
};

// std140 mirror of ojmShadowTex/Notex.frag binding 2 set 2: the SHADOWED rows' block
// ShadowMatrix must be the very value used for the SELF_DEPTH pass
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
