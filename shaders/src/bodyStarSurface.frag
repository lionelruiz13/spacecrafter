//
// bodyStarSurface - fragment stage of the near-surface star family's
// photosphere (B12; design note claude/b12-design.md S5.1).
//
// A star's photosphere EMITS. Old-path anchor: at disc centre this is
// body_sun.frag exactly (the texel, unmodified) - the old Sun's whole fragment
// stage. What is added is the angular dependence the old path had no term for:
//
//   Plane-parallel grey atmosphere, radiative equilibrium, LTE (S = J),
//   Eddington closure K = J/3  =>  S(tau) = (3F/4pi)(tau + 2/3).
//   Emergent intensity  I(0,mu) = INT_0^inf S(tau) e^{-tau/mu} dtau/mu
//                               = (3F/4pi)(mu + 2/3)
//   (using INT tau e^{-tau/mu} dtau = mu^2 and INT e^{-tau/mu} dtau = mu),
//   normalised to the disc centre:   L(mu) = (3*mu + 2)/5.
//
// DERIVED, not fitted and not recalled: the law has no measured coefficient in
// it, which is what keeps it clear of the "physical values never from memory"
// red line (INTENT S11.51(d)). A band-specific measured coefficient (the
// 1 - u(1-mu) family) would be a refinement needing a cited source.
//
#version 420

layout (binding=1) uniform sampler2D mapTexture;

layout (location=0) in vec2 TexCoord;
layout (location=1) in vec3 Position;
layout (location=2) in vec3 Normal;

layout (location=0) out vec4 FragColor;

void main(void)
{
	// mu = cosine between the outward surface normal and the direction from the
	// surface point to the observer; the eye is the origin of eye space, so
	// that direction is -Position. Exact at any distance (no far-field
	// assumption) and correct under body scaling, since both varyings come from
	// the drawn geometry. Clamped: back faces are culled, but the silhouette
	// row of a coarse LOD sphere can land marginally negative.
	float mu = max(dot(normalize(Normal), normalize(-Position)), 0.0);
	vec3 color = texture(mapTexture, TexCoord).rgb;
	FragColor = vec4(color * ((3.0 * mu + 2.0) * 0.2), 1.0);
}
