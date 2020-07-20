#include <GL/glew.h>
#include "mainModule/sdl_facade.hpp"
#include "tools/log.hpp"
#include "tools/stateGL.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"
#include "tools/Renderer.hpp"
#include "signal.h"
#include "tools/vecmath.hpp"

bool opened = true;

#define width 800
#define height 600

constexpr float DEG2RAD = 3.14159265358979323 / 180.;
constexpr float RATIO = (float) width / (float) height;

void sigTerm(int sig)
{
    (void) sig;
    opened = false;
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
    cLog *Log = cLog::get();
    SDLFacade window;

    signal(SIGINT, sigTerm);
    signal(SIGTERM, sigTerm);
    Log->openLog(LOG_FILE::INTERNAL, "spacecrafter");
    window.initSDL();
    window.createWindow("Experiment", width, height, 0, 3, false, "~/.spacecrafter/data/icon.bpm");

    // varray->registerVertexBuffer(BufferType::SHAPE, BufferAccess::STATIC);

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

    MyObj ground(std::vector<float>({-100,0,-100, 100,0,-100, 100,0,100, 100,0,100, -100,0,100, -100,0,-100}), std::vector<float>({1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1, 1,1,1}));

    std::cout << "1\n";
    auto shader = std::make_unique<shaderProgram>();
    shader->init("experiment.vert", "experiment.frag");

    std::cout << "2\n";
    const auto proj = Mat4f::perspective(70, RATIO, 0.1, 100.);
    auto cam = Vec3f(0, 4, -4);
    auto target = Vec3f(0, 0, 0);
    auto up = Vec3f(0, 1, 0);

    Mat4f view;
    auto mat = Mat4f::scaling(0.7).translation(Vec3f(-0.5, 0, -0.5));
    const auto rotate = Mat4f::yawPitchRoll(1.2, 0.6, 0);

    int fixer = 0;

    std::cout << "3\n";
    auto varray = std::make_unique<VertexArray>();
    varray->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
    varray->registerVertexBuffer(BufferType::COLOR, BufferAccess::STATIC);

    varray->fillVertexBuffer(BufferType::POS3D, MyObj::vertices);
    varray->fillVertexBuffer(BufferType::COLOR, MyObj::colors);

    std::cout << "4\n";
    StateGL::disable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);
    StateGL::enable(GL_DEPTH_TEST);

    std::cout << "start\n";

    SDL_Event event;

    while (opened) {
        cam = rotate * cam;
        view = Mat4f::lookAt(cam, target, up);
        Renderer::clearBuffer();
        //std::cout << "\ec";
        //view.print();
        /*
        proj.print();
        mat.print();
        (proj * Mat4f::lookAt(cam, target, up)).print(); //*/
        shader->use();
        shader->setUniform("MVP", proj * view * mat);
        Renderer::drawArrays(shader.get(), varray.get(), GL_TRIANGLES, 0, MyObj::nbElements * 3);
        window.glSwapWindow();
    }

    Log->close();
    return 0;
}
