// shadowRing.frag - TEXTURED_ANNULUS word (G8, ShadowService): graded light
// coverage of a planar ring. r = radial distance in units of the OUTER
// radius; the band [innerRatio, 1] samples the radial ring texture's alpha
// (opacity = coverage; the old-parity darkening DEPTH lives in the entry's
// absorbtion, not here - mix(1.0, 0.3, a) == 1 - a*0.7, shadow-paths.md B4).
// u = 1 - (r - ri)/(1 - ri): the old body_ringed.frag:52 orientation, OUTER
// edge at u=0 - the texture is authored that way.
#version 450

layout (binding=0, set=1) uniform sampler2D ringTexture;

layout (push_constant) uniform pc {
    float innerRatio;
};

layout (location=0) in vec2 plane;

layout (location=0) out float coverage;

void main()
{
    float r = length(plane);
    if (r < innerRatio || r > 1.0) {
        coverage = 0.0;
    } else {
        coverage = texture(ringTexture, vec2(1.0 - (r - innerRatio) / (1.0 - innerRatio), 0.5)).a;
    }
}
