// Received-shadow DECLARATIONS - single authority (I2) for every receiver
// fragment; pairs with receivedShadows.glsl (the sampling function). Include
// BEFORE declaring the family's uniform block; the block itself stays
// per-family (binding numbers and sibling fields differ), but must expose
// EXACTLY these member names for the function include:
//   vec4 shadowRow0, shadowRow1;  int nbShadowingBodies;
//   ShadowingBody shadowingBodies[MAX_SHADOW_CASTERS];
// plus a sampler2DArray named bodyShadows (any binding).
// CPU mirrors: bodyShaderInterface.hpp (meshFrag/rayMarchFrag/ojmShadowBlock).
#ifndef RECEIVED_SHADOWS_DECL
#define RECEIVED_SHADOWS_DECL

#define MAX_SHADOW_CASTERS 8

struct ShadowingBody {
	vec4 posRadius;      // xy = caster center in sun-frame (rel. receiver), z = disc radius
	vec4 absorbtionIdx;  // rgb = caster shadow absorption, w = layer index
	vec4 clip;           // half-space gate: apply iff dot(P, xyz) + w <= 0 ((0,0,0,-1) = always; planar casters)
};

#endif
