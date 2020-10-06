#ifndef RESOURCE_TRACKER_HPP
#define RESOURCE_TRACKER_HPP

#include <vector>
#include <memory>

class VertexArray;
class Pipeline;
class PipelineLayout;
class Texture;
class Uniform;
class Buffer;
class Set;

/**
 * \class ResourceTracker
 * \brief This class own resource to release them before destroying Vulkan device.
 * Use this instead of static member with smart pointer.
 */
class ResourceTracker
{
public:
    ResourceTracker();
    ~ResourceTracker();
    VertexArray *track(VertexArray *toTrack);
    Pipeline *track(Pipeline *toTrack);
    PipelineLayout *track(PipelineLayout *toTrack);
    Texture *track(Texture *toTrack);
    Uniform *track(Uniform *toTrack);
    Buffer *track(Buffer *toTrack);
    Set *track(Set *toTrack);
    Pipeline *trackArray(Pipeline *toTrack);
private:
    std::vector<std::unique_ptr<VertexArray>> vertexArray;
    std::vector<std::unique_ptr<Pipeline>> pipeline;
    std::vector<std::unique_ptr<PipelineLayout>> pipelineLayout;
    std::vector<std::unique_ptr<Texture>> texture;
    std::vector<std::unique_ptr<Uniform>> uniform;
    std::vector<std::unique_ptr<Buffer>> buffer;
    std::vector<std::unique_ptr<Set>> set;
    std::vector<Pipeline *> pipelineArray;
};

#endif /* end of include guard: RESOURCE_TRACKER_HPP */
