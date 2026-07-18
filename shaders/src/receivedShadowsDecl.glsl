// Received-shadow DECLARATIONS - single authority (I2) for every receiver
// fragment; pairs with receivedShadows.glsl (the sampling function). Include
// BEFORE declaring the family's uniform block; the block itself stays
// per-family (binding numbers and sibling fields differ), but must expose
// EXACTLY these member names for the function include:
//   int nbShadowingBodies;
//   ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
// plus a sampler2DArray named bodyShadows (any binding).
// CPU mirrors: bodyShaderInterface.hpp (meshFrag/rayMarchFrag/ojmShadowBlock).
//
// The sun-frame projection rows live PER ENTRY (row0/row1) since 2026-07-18
// [vixy]: an entry's fold depends on the LIGHT that casts it - self-contained
// entries are what let multiple light sources land in the selection alone,
// with receivers untouched. Folded receivers (model-space P) fold rows AND
// clip through the same model->eye map (meshShadowFill.hpp fillFoldedShadows)
// - both are affine forms over the same P.
#ifndef RECEIVED_SHADOWS_DECL
#define RECEIVED_SHADOWS_DECL

#define MAX_SHADOW_CASTERS 8

struct ShadowingBody {
	vec4 posRadius;      // xy = caster center in sun-frame (rel. receiver), z = disc radius
	vec4 absorbtionIdx;  // rgb = TRANSMISSION absorption aT (solid casters 1 -
	                     // they block direct light fully; rings = material
	                     // absorption, light passes THROUGH), w = layer index
	vec4 clip;           // half-space gate: apply iff dot(P, xyz) + w <= 0 ((0,0,0,-1) = always; planar casters)
	vec4 row0;           // sun-frame rows of THIS entry: shadowPos =
	vec4 row1;           //   (dot(row0.xyz, P) + row0.w, dot(row1.xyz, P) + row1.w)
	vec4 glow;           // rgb = REFRACTION glow gR: light the caster's
	                     // atmosphere bends into its TRUE UMBRA (Earth red -
	                     // physically the refracted radiance; photography-
	                     // calibration criterion [vixy: 2026-07-18]). Applies
	                     // by the layer's G (umbra) channel only. 0 for rings
	                     // and airless bodies.
};

#endif
