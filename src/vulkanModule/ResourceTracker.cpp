#include "ResourceTracker.hpp"
#include "VertexArray.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "Texture.hpp"
#include "Uniform.hpp"
#include "Buffer.hpp"

ResourceTracker::ResourceTracker() {}

ResourceTracker::~ResourceTracker() {}

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
