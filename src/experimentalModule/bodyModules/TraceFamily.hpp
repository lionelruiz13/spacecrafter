#ifndef TRACE_FAMILY_HPP_
#define TRACE_FAMILY_HPP_

#include "experimentalModule/PipelineFamily.hpp"
#include "tools/vecmath.hpp"

class Renderer;
class PipelineLayout;

struct TraceInfo {           // push-constant block of body_depth_trace.vert
    Mat4f ModelViewMatrix;
    Vec3f clipping_fov;      // of the ORBIT depth range
    float planetScaledRadius;
    float planetOneMinusOblateness;
};

// Trace prepass, bodies write depth to hide the orbit lines
namespace TraceFamily {
    const PipelineFamily &sphere();
    // Read ringVertex at first call only, it must stay valid
    const PipelineFamily &ring(VertexArray *ringVertex);
}

#endif /* end of include guard: TRACE_FAMILY_HPP_ */
