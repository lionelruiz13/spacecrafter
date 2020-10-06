#include "ResourceTracker.hpp"
#include "VertexArray.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "Texture.hpp"
#include "Uniform.hpp"
#include "Buffer.hpp"
#include "Set.hpp"

ResourceTracker::ResourceTracker() {}

ResourceTracker::~ResourceTracker()
{
    for (auto p : pipelineArray) {
        delete[] p;
    }
}

VertexArray *ResourceTracker::track(VertexArray *toTrack)
{
    vertexArray.emplace_back(toTrack);
    return toTrack;
}

Pipeline *ResourceTracker::track(Pipeline *toTrack)
{
    pipeline.emplace_back(toTrack);
    return toTrack;
}

PipelineLayout *ResourceTracker::track(PipelineLayout *toTrack)
{
    pipelineLayout.emplace_back(toTrack);
    return toTrack;
}

Texture *ResourceTracker::track(Texture *toTrack)
{
    texture.emplace_back(toTrack);
    return toTrack;
}

Uniform *ResourceTracker::track(Uniform *toTrack)
{
    uniform.emplace_back(toTrack);
    return toTrack;
}

Buffer *ResourceTracker::track(Buffer *toTrack)
{
    buffer.emplace_back(toTrack);
    return toTrack;
}

Set *ResourceTracker::track(Set *toTrack)
{
    set.emplace_back(toTrack);
    return toTrack;
}

Pipeline *ResourceTracker::trackArray(Pipeline *toTrack)
{
    pipelineArray.push_back(toTrack);
    return toTrack;
}

// temporary, just for test
#include "Context.hpp"
ThreadContext *getContext() {
    static ThreadContext context{};
    return &context;
}
