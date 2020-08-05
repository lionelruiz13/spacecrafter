#include <GL/glew.h>
#include "Vulkan.hpp"
#include "mainModule/sdl_facade.hpp"
#include "tools/log.hpp"
#include "tools/stateGL.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"
#include "tools/Renderer.hpp"
#include "signal.h"

bool opened = true;

#define width 600
#define height 600

constexpr float DEG2RAD = 3.14159265358979323 / 180.;
constexpr float RATIO = (float) width / (float) height;

static void sigTerm(int sig)
{
    (void) sig;
    opened = false;
    std::cout << "\rExit\n";
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

int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *VkWindow = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

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

    std::cout << "Initializing shader...\n";
    auto shader = std::make_unique<shaderProgram>();
    shader->init("experiment.vert", "experiment.geom", "experiment.frag");
    std::cout << "Shader initialized.\n";

    auto clipping_fov = Vec3f(0.1, 100, 50);

    auto cam = Vec3f(0, 4, -4);
    auto target = Vec3f(0, 1, 0);
    auto up = Vec3f(0, 1, 0);

    auto mat = Mat4f::scaling(0.7).translation(Vec3f(-0.5, 0, -0.5));
    const auto rotate = Mat4f::yawPitchRoll(1.2, 0.6, 0);

    auto varray = std::make_unique<VertexArray>();
    varray->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
    varray->registerVertexBuffer(BufferType::COLOR, BufferAccess::STATIC);

    varray->fillVertexBuffer(BufferType::POS3D, MyObj::vertices);
    varray->fillVertexBuffer(BufferType::COLOR, MyObj::colors);

    StateGL::disable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);
    StateGL::enable(GL_DEPTH_TEST);

    Mat4f view;

    shader->setUniformLocation({"MV", "clipping_fov"});
    {
        int nb_loops = 1000;
        Vulkan vulkan("mini_projet", "No Engine", VkWindow);
        while (opened && nb_loops-- > 0) {
            cam = rotate * cam;
            view = Mat4f::lookAt(cam, target, up);
            Renderer::clearBuffer();
            shader->use();
            shader->setUniform("MV", view * mat);
            shader->setUniform("clipping_fov", clipping_fov);
            Renderer::drawArrays(shader.get(), varray.get(), GL_TRIANGLES, 0, MyObj::nbElements * 3);
            window.glSwapWindow();
            vulkan.drawFrame();
        }
    }
    glfwDestroyWindow(VkWindow);
    glfwTerminate();
    Log->close();
    return 0;
}
