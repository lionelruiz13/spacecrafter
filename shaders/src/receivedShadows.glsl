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

// Physical-sharp composition (2026-07-18 [vixy]): per entry, with c = layer R
// (mean sun-occlusion) and u = layer G (true-umbra fraction, u <= c):
//   T = 1 - c*aT + u*gR
// - direct light scales by the UNOCCLUDED sun fraction through the caster's
//   material transmission (solid: aT=1 -> neutral (1-c) ramp across penumbra
//   AND antumbra - no chromatic tint where direct sunlight remains; rings:
//   aT = material absorption, the legacy 1-c*a transmission physics);
// - the chromatic term is the caster's atmospheric REFRACTION glow gR,
//   present only where the sun is fully occluded (u): Earth's red umbra,
//   physically the light bent around the limb - absent in penumbra/antumbra
//   where it is drowned by direct light.
// Bounds: u <= c and gR <= 1 give 0 <= T <= 1 per entry; products commute.
// Umbra floor = gR exactly (== the legacy 1-a at c=1: umbra look unchanged);
// airless casters (gR=0) reduce EXACTLY to the previous formula.
vec3 computeReceivedShadowing(vec3 P)
{
	vec3 shadowing = vec3(1.0);
	for (int i = 0; i < nbShadowingBodies; ++i) {
		vec2 shadowPos = vec2(dot(shadowingBodies[i].row0.xyz, P) + shadowingBodies[i].row0.w,
		                      dot(shadowingBodies[i].row1.xyz, P) + shadowingBodies[i].row1.w);
		vec2 tmp = (shadowPos - shadowingBodies[i].posRadius.xy) / shadowingBodies[i].posRadius.z;
		if (dot(tmp, tmp) < 1.0 && dot(P, shadowingBodies[i].clip.xyz) + shadowingBodies[i].clip.w <= 0.0) {
			vec2 cov = textureLod(bodyShadows, vec3(tmp * 0.5 + 0.5, shadowingBodies[i].absorbtionIdx.w), 0.0).rg;
			shadowing *= vec3(1.0) - cov.r * shadowingBodies[i].absorbtionIdx.rgb
			                       + cov.g * shadowingBodies[i].glow.rgb;
		}
	}
	return shadowing;
}

#endif
