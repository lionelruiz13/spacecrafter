// Received-shadow SAMPLING - single authority (I2) for the per-entry
// application loop, extracted 2026-07-16 when the OJM receiver would have
// been the 10th copy (the 9-frag textureLod UB fix demonstrated the drift
// cost of the copies). Requires receivedShadowsDecl.glsl + the family block
// + the bodyShadows sampler declared BEFORE this include (see Decl header).
//
// P = the receiver's surface point in WHATEVER frame the entries were folded
// to (eye space for disc receivers, model space for ray-march/OJM receivers
// - the fill folds rows and clip through the same map, so caller consistency
// is by construction). Since the per-entry-rows restructure (2026-07-18) the
// sun-frame projection happens HERE, per entry: one P serves rows and clip -
// both are affine forms over the same point.
// Returns the per-channel transmission product (1 - cov_i * absorbtion_i)
// over the applicable entries - multiplication commutes, so per-module
// layers compose exactly like one combined caster map.
//
// textureLod, NOT texture: implicit derivatives are UNDEFINED in the
// non-uniform flow every caller has (zero reads on NVIDIA, 2026-07-16);
// the layer array is single-mip, lod 0 is exact in any flow.
#ifndef RECEIVED_SHADOWS_FN
#define RECEIVED_SHADOWS_FN

vec3 computeReceivedShadowing(vec3 P)
{
	vec3 shadowing = vec3(1.0);
	for (int i = 0; i < nbShadowingBodies; ++i) {
		vec2 shadowPos = vec2(dot(shadowingBodies[i].row0.xyz, P) + shadowingBodies[i].row0.w,
		                      dot(shadowingBodies[i].row1.xyz, P) + shadowingBodies[i].row1.w);
		vec2 tmp = (shadowPos - shadowingBodies[i].posRadius.xy) / shadowingBodies[i].posRadius.z;
		if (dot(tmp, tmp) < 1.0 && dot(P, shadowingBodies[i].clip.xyz) + shadowingBodies[i].clip.w <= 0.0) {
			float coverage = textureLod(bodyShadows, vec3(tmp * 0.5 + 0.5, shadowingBodies[i].absorbtionIdx.w), 0.0).r;
			shadowing *= vec3(1.0) - coverage * shadowingBodies[i].absorbtionIdx.rgb;
		}
	}
	return shadowing;
}

#endif
