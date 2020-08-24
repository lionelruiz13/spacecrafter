#include <GL/glew.h>
//#include "Vulkan.hpp"
#include "mainModule/sdl_facade.hpp"
#include "tools/log.hpp"
#include "signal.h"
#include "VirtualSurface.hpp"
#include "Pipeline.hpp"
#include "PipelineLayout.hpp"
#include "CommandMgr.hpp"
#include "VertexBuffer.hpp"
#include "Uniform.hpp"
#include "UniformSet.hpp"
#include <thread>

//*
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct uniformBufferObject {
    Mat4f MV;
    Vec3f clipping_fov;
};

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
        return attributeDescriptions;
    }
};

struct Vertex2 {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex2);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex2, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex2, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex2, texCoord);
        return attributeDescriptions;
    }
};

const std::vector<Vertex> vertices = {
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

bool opened = true;

#define width 600
#define height 600

constexpr float DEG2RAD = 3.14159265358979323 / 180.;
constexpr float RATIO = (float) width / (float) height;

static void sigTerm(int sig)
{
    static int count = 0;
    (void) sig;
    opened = false;
    std::cout << "\rExit\n";
    if (++count >= 3) {
        throw std::runtime_error("Multiple sigTerm received before termination.");
    }
}

class MyObj {
public:
    MyObj(const std::vector<float> &_vertices, const std::vector<float> &_colors) {
        assert(_vertices.size() == _colors.size());
        assert(_vertices.size() % 9 == 0);
        vertices.insert(vertices.end(), _vertices.begin(), _vertices.end());
        colors.insert(colors.end(), _colors.begin(), _colors.end());
        nbElements = vertices.size() / 9;
        std::cout << "ELEMENTS : " << nbElements << std::endl;
    };
    static int nbElements;
    static std::vector<float> vertices;
    static std::vector<float> colors;
};

int MyObj::nbElements = 0;
std::vector<float> MyObj::vertices;
std::vector<float> MyObj::colors;

static void cubeFunc(VirtualSurface *surface, bool *opened, Mat4f rotate, Vec3f cam, Vec3f target, Vec3f up, Vec3f clipping_fov)
{
    Mat4f view;
    auto mat = Mat4f::scaling(0.7).translation(Vec3f(-0.5, 0, -0.5));

    std::vector<VkVertexInputAttributeDescription> vec;
    {
        auto tmp = Vertex2::getAttributeDescriptions();
        vec.assign(tmp.begin(), tmp.end());
    }
    VertexBuffer vertex(surface, 42, Vertex2::getBindingDescription(), vec);

    glm::vec3 *pVertices = (glm::vec3 *) MyObj::vertices.data();
    glm::vec3 *pColors = (glm::vec3 *) MyObj::colors.data();
    Vertex2 *pVertex = (Vertex2 *) vertex.data;
    for (int i = 0; i < 42; i++) {
        pVertex->pos = *pVertices;
        pVertex->color = *pColors;
        pVertex++;
        pVertices++;
        pColors++;
    }
    vertex.update();

    Uniform ubo(surface, sizeof(uniformBufferObject), VK_SHADER_STAGE_GEOMETRY_BIT);
    uniformBufferObject *uboData = (uniformBufferObject *) ubo.data;

    PipelineLayout pipelineLayout(surface);
    pipelineLayout.setUniformLocation(VK_SHADER_STAGE_VERTEX_BIT, 0);
    pipelineLayout.build();

    UniformSetMgr setMgr(surface, 1);

    UniformSet set(surface, &setMgr, &pipelineLayout);
    set.bindUniform(&ubo, 0);

    Pipeline pipeline(surface, &pipelineLayout);
    pipeline.bindVertex(vertex, 0);
    pipeline.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    pipeline.setDepthStencilMode(VK_TRUE, VK_TRUE);
    pipeline.setBlendMode(BLEND_SRC_ALPHA);
    pipeline.bindShader("shader/experiment_vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipeline.bindShader("shader/experiment_geom.spv", VK_SHADER_STAGE_GEOMETRY_BIT);
    pipeline.bindShader("shader/experiment_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline.build();

    CommandMgr cmdMgr(surface, 1);
    cmdMgr.init(0);
    cmdMgr.beginRenderPass(renderPassType::PRESENT);
    cmdMgr.bindPipeline(&pipeline);
    cmdMgr.bindVertex(&vertex, 0);
    cmdMgr.bindUniformSet(&pipelineLayout, &set);
    cmdMgr.draw(42);
    cmdMgr.endRenderPass();
    cmdMgr.compile();
    cmdMgr.setSubmission(0);

    surface->finalize();
    while (*opened) {
        surface->acquireNextFrame();
        cam = rotate * cam;
        view = Mat4f::lookAt(cam, target, up);
        uboData->MV = (view * mat);
        uboData->clipping_fov = clipping_fov;
        surface->submitFrame();
    }
    surface->waitEmpty();
}

static void squareFunc(VirtualSurface *surface, bool *opened, Mat4f rotate, Vec3f cam, Vec3f target, Vec3f up, Vec3f clipping_fov)
{
    Mat4f view;

    std::vector<VkVertexInputAttributeDescription> vec;
    {
        auto tmp = Vertex::getAttributeDescriptions();
        vec.assign(tmp.begin(), tmp.end());
    }
    VertexBuffer vertex(surface, vertices.size(), Vertex::getBindingDescription(), vec);
    memcpy(vertex.data, vertices.data(), sizeof(Vertex) * vertices.size());
    vertex.update();

    PipelineLayout pipelineLayout(surface);
    pipelineLayout.build();

    Pipeline pipeline(surface, &pipelineLayout);
    pipeline.bindVertex(vertex, 0);
    pipeline.setTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
    pipeline.setDepthStencilMode(VK_TRUE, VK_TRUE);
    pipeline.setBlendMode(BLEND_SRC_ALPHA);
    pipeline.bindShader("shader/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    pipeline.bindShader("shader/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
    pipeline.build();

    CommandMgr cmdMgr(surface, 1);
    cmdMgr.init(0);
    cmdMgr.beginRenderPass(renderPassType::CLEAR);
    cmdMgr.bindPipeline(&pipeline);
    cmdMgr.bindVertex(&vertex, 0);
    cmdMgr.draw(4);
    cmdMgr.endRenderPass();
    cmdMgr.compile();
    cmdMgr.setSubmission(0);

    surface->finalize();
    while (*opened) {
        surface->acquireNextFrame();
        cam = rotate * cam;
        view = Mat4f::lookAt(cam, target, up);
        surface->submitFrame();
    }
    surface->waitEmpty();
}

int main()
{
    cLog *Log = cLog::get();
    SDLFacade window;

    signal(SIGINT, sigTerm);
    signal(SIGTERM, sigTerm);
    Log->openLog(LOG_FILE::INTERNAL, "spacecrafter");
    window.initSDL();
    window.createWindow("Experiment", width, height, 24, 3, false, "~/.spacecrafter/data/icon.bpm");

    MyObj cube(std::vector<float>({
        0,0,0, 1,0,0, 1,1,0, 1,1,0, 0,1,0, 0,0,0,
        0,0,0, 0,1,0, 0,1,1, 0,1,1, 0,0,1, 0,0,0,
        0,0,0, 0,0,1, 1,0,1, 1,0,1, 1,0,0, 0,0,0,

        1,1,1, 0,1,1, 0,0,1, 0,0,1, 1,0,1, 1,1,1,
        1,1,1, 1,0,1, 1,0,0, 1,0,0, 1,1,0, 1,1,1,
        1,1,1, 1,1,0, 0,1,0, 0,1,0, 0,1,1, 1,1,1
    }), std::vector<float>({
        1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0, 1,0,0,
        0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0, 0,1,0,
        0,0,1, 0,0,1, 0,0,1, 0,0,1, 0,0,1, 0,0,1,

        0,1,1, 0,1,1, 0,1,1, 0,1,1, 0,1,1, 0,1,1,
        1,0,1, 1,0,1, 1,0,1, 1,0,1, 1,0,1, 1,0,1,
        1,1,0, 1,1,0, 1,1,0, 1,1,0, 1,1,0, 1,1,0
    }));

    MyObj ground(std::vector<float>({-65,0,-65, 65,0,-65, 65,0,65, 65,0,65, -65,0,65, -65,0,-65}), std::vector<float>({1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1}));

    auto clipping_fov = Vec3f(0.1, 100, 50);

    auto cam = Vec3f(0, 4, -4);
    auto target = Vec3f(0, 1, 0);
    auto up = Vec3f(0, 1, 0);

    auto mat = Mat4f::scaling(0.7).translation(Vec3f(-0.5, 0, -0.5));
    const auto rotate = Mat4f::yawPitchRoll(1.2, 0.6, 0);

    Mat4f view;

    // Engage vulkan engine
    bool opened2 = true;
    Vulkan *vulkan = new Vulkan("mini_projet", "No Engine", window.getWindow(), 2);
    std::thread thread(squareFunc, vulkan->getVirtualSurface(), &opened2, rotate, cam, target, up, clipping_fov);
    std::thread thread2(cubeFunc, vulkan->getVirtualSurface(), &opened2, rotate, cam, target, up, clipping_fov);
    int nb_loops = 1000;
    vulkan->finalize();
    while (opened && nb_loops-- > 0) {
        cam = rotate * cam;
        view = Mat4f::lookAt(cam, target, up);
        vulkan->sendFrame();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    opened2 = false;
    vulkan->sendFrame();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    vulkan->sendFrame();
    thread.join();
    thread2.join();
    delete vulkan;
    Log->close();
    return 0;
}
