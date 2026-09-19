#ifndef TRACE_FAMILY_HPP_
#define TRACE_FAMILY_HPP_

#include "experimentalModule/PipelineFamily.hpp"
#include "tools/vecmath.hpp"

class Renderer;
class PipelineLayout;

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
