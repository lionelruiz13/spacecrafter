#ifndef TRACE_FAMILY_HPP_
#define TRACE_FAMILY_HPP_

#include "experimentalModule/PipelineFamily.hpp"
#include "tools/vecmath.hpp"

class Renderer;
class PipelineLayout;

struct TraceInfo {           // push-constant block of body_depth_trace.vert
    Mat4f ModelViewMatrix;   // offset 0
    Vec3f clipping_fov;      // offset 64 (the ORBIT range this frame)
    float planetScaledRadius;// offset 76
    float planetOneMinusOblateness; // offset 80
};

// TRACE prepass: a body's solid geometry writes its own depth under the orbit-union depth range
// (Renderer::getOrbitDepthBucket), so the orbit line, depth-tested in the same range, vanishes behind it
namespace TraceFamily {
    // Disc-hole family of sphere bodies (ojmVertexArray, triangle list, position-only)
    const PipelineFamily &sphere();
    // Ring-annulus family (triangle strip, position-only)
    // ringVertex = the ring module's VertexArray, read at first request only; application-lifetime
    const PipelineFamily &ring(VertexArray *ringVertex);
}

#endif /* end of include guard: TRACE_FAMILY_HPP_ */
