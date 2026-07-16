// shadow_shape.frag - OPAQUE_MESH word of the typed silhouette vocabulary
// (ShadowService): the caster mesh writes full coverage (light transmission 0)
// into the R8 shape target. Composited coverage-over; the graded words (ring
// alpha) write < 1 - the stencil-era target could only say 0/1, which is why
// it could not carry G8 (shadow-paths.md B3, preconditions shifted).
#version 450

layout (location=0) out float coverage;

void main()
{
    coverage = 1.0;
}
