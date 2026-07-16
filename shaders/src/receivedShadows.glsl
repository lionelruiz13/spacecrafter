// Received-shadow SAMPLING - single authority (I2) for the per-entry
// application loop, extracted 2026-07-16 when the OJM receiver would have
// been the 10th copy (the 9-frag textureLod UB fix demonstrated the drift
// cost of the copies). Requires receivedShadowsDecl.glsl + the family block
// + the bodyShadows sampler declared BEFORE this include (see Decl header).
//
// shadowPos = the surface point projected by the two folded sun-frame rows
// (ShadowProjection.hpp); P = the point the CLIP half-space gates on, in
// WHATEVER frame the entries' clip planes were folded to (eye space for disc
// receivers, model space for ray-march/OJM receivers - the fill folds rows
// and clip through the same map, so caller consistency is by construction).
// Returns the per-channel transmission product (1 - cov_i * absorbtion_i)
// over the applicable entries - multiplication commutes, so per-module
// layers compose exactly like one combined caster map.
//
// textureLod, NOT texture: implicit derivatives are UNDEFINED in the
// non-uniform flow every caller has (zero reads on NVIDIA, 2026-07-16);
// the layer array is single-mip, lod 0 is exact in any flow.
#ifndef RECEIVED_SHADOWS_FN
#define RECEIVED_SHADOWS_FN

vec3 computeReceivedShadowing(vec2 shadowPos, vec3 P)
{
	vec3 shadowing = vec3(1.0);
	for (int i = 0; i < nbShadowingBodies; ++i) {
		vec2 tmp = (shadowPos - shadowingBodies[i].posRadius.xy) / shadowingBodies[i].posRadius.z;
		if (dot(tmp, tmp) < 1.0 && dot(P, shadowingBodies[i].clip.xyz) + shadowingBodies[i].clip.w <= 0.0) {
			float coverage = textureLod(bodyShadows, vec3(tmp * 0.5 + 0.5, shadowingBodies[i].absorbtionIdx.w), 0.0).r;
			shadowing *= vec3(1.0) - coverage * shadowingBodies[i].absorbtionIdx.rgb;
		}
	}
	return shadowing;
}

#endif
