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
	int nbShadowingBodies;
	int _pad[3];
	struct ShadowingBody {
		Vec4f posRadius;      // xy = caster center in sun-frame (rel. receiver), z = disc radius, w unused
		Vec4f absorbtionIdx;  // rgb = TRANSMISSION absorption aT (solid 1; rings = material), w = layer index
		Vec4f clip;           // eye-space half-space gate: apply iff dot(P, xyz) + w <= 0
		                      // ((0,0,0,-1) = always; planar casters - ShadowProjection.hpp)
		// Sun-frame projection rows of THIS entry, receiver-folded
		// (ShadowProjection.hpp): shadowPos = (dot(row0.xyz, P) + row0.w,
		// dot(row1.xyz, P) + row1.w). PER-ENTRY [vixy: 2026-07-18] - the fold
		// depends on the entry's LIGHT; self-contained entries are what let
		// multiple light sources land in the selection alone, receivers
		// untouched. Folded receivers fold rows and clip through the SAME map.
		Vec4f row0, row1;
		Vec4f glow;           // rgb = refraction glow gR (umbra-only chroma,
		                      // physical-sharp composition [vixy: 2026-07-18]
		                      // - receivedShadows.glsl derivation), w unused
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
// bodyShader.hpp:232-241). ALSO read by bodyRayMarch.frag since the 5.29 fix
// (true ray-hit gl_FragDepth): the frag re-declares this block verbatim, so
// any field added here must be added there too - the binding is
// VERTEX|FRAGMENT in the contract. ModelViewMatrix = m * scale(1,1,oneMinusOblateness)
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
// old ShadowFrag with the mat3 ShadowMatrix REPLACED by the S5 sun-frame
// rows, per-entry and pre-folded through the model matrix so shadowPos lands
// in eye-space AU and caster posRadius entries are consumed unchanged (no
// initialRadius normalization - it existed because the old frames mixed unit
// systems, shadow-paths.md E "Units").
// Entry-row fold (fillFoldedShadows): rowL.xyz = finalRadius *
// (row.xyz . linearColumns(MV)), rowL.w = row.w + dot(row.xyz,
// MV.translation), so dot(rowL.xyz, samplePosUnit) + rowL.w == S5 shadowPos
// of the eye-space ground point (height term neglected, old-parity class).
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

// ---- RING family (row 4, 2026-07-18) ---------------------------------------
// bodyRing.vert/.frag - the ring_planet.* port with generalized receive
// (RingModule.hpp is the module contract; this block is the GPU interface).

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

// ---- OJM family (row 3, 2026-07-16) ----------------------------------------
// body_artificial.vert/geom REUSED VERBATIM; plain rows reuse
// body_artificial_tex/notex.frag verbatim, shadowed rows are the
// ojmShadowTex/Notex ports. One layout for all four rows (INTENT 10.3 rule
// 4); binding 2 carries ojmLight on the plain rows and ojmShadowBlock on the
// shadowed rows - same binding TYPE, different module-owned buffer, hence the
// module's two Sets (the old set/ext->shadowSet pair, ported).

// std140 mirror of body_artificial.vert binding 0 set 2 ("custom" block:
// mat3 NormalMatrix). Fill via Mat4f::setMat3 of the eye-space model matrix
// (rotation part - old drawBody matrix.setMat3(uVert->normal)).
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

// std140 mirror of ojmShadowTex/Notex.frag binding 2 set 2 (ojmShadowBlock)
// - the SHADOWED rows' block. ShadowMatrix is the SAME value production used
// for the SELF_DEPTH pass (single-computation consistency,
// ShadowService.hpp); per-entry rows/clip fold through the model->eye map
// (meshShadowFill.hpp, fillFoldedShadows).
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
