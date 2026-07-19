#ifndef TRACE_FAMILY_HPP_
#define TRACE_FAMILY_HPP_

#include "experimentalModule/PipelineFamily.hpp"
#include "tools/vecmath.hpp"

class Renderer;
class PipelineLayout;

// The TRACE prepass service (INTENT §12 row 8, §3.1 drawing-type 4). A body's
// solid geometry writes ITS OWN disc into the depth buffer under the orbit-
// union depth range (Renderer::getOrbitDepthBucket) so the orbit LINE, drawn
// depth-tested against it in the same range, vanishes where a body hides it.
// New-path analog of the old drawOrbit(cmdBodyDepth,…)/getShaderDepthTrace()
// pair (solarsystem_display.cpp:312-336; bodyShader.cpp:368-386).
//
// Two families because the geometry differs (vertex format + topology), the
// old-path split too (one shared depthTrace layout, distinct pipelines - the
// sphere depthTrace.pipeline vs Ring::pipelineDepthTrace, ring.cpp:133-143):
//  - sphere: the ojm sphere (mesh/OJM bodies), triangle list, position-only.
//  - ring:   the ring annulus (RING module), triangle strip, position-only.
// Both use body_depth_trace.vert (no fragment - depth-only) and the same
// push-constant block, spec-const 8 registry-injected (INTENT §11.33).
//
// A drawTrace hook binds its family (bind-and-record, redundant per-body binds
// are cheap - a handful of on-screen bodies) and draws its own geometry.
struct TraceInfo {           // == old depthTraceInfo (bodyShader.hpp:171-176)
    Mat4f ModelViewMatrix;   // offset 0
    Vec3f clipping_fov;      // offset 64 (the ORBIT range this frame)
    float planetScaledRadius;// offset 76
    float planetOneMinusOblateness; // offset 80
};

namespace TraceFamily {
    // Disc-hole family for sphere bodies (ojmVertexArray, triangle list,
    // tex/normal vertex entries stripped - old removeVertexEntry(1)/(2)).
    const PipelineFamily &sphere();
    // Ring-annulus family (row-4 RING TRACE going live). vertex = the ring
    // module's VertexArray (passed at first request; application-lifetime).
    const PipelineFamily &ring(VertexArray *ringVertex);
}

#endif /* end of include guard: TRACE_FAMILY_HPP_ */
