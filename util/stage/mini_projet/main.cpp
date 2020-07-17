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

void sigTerm(int sig)
{
    (void) sig;
    opened = false;
}

int main()
{
    cLog *Log = cLog::get();
    SDLFacade window;

    signal(SIGINT, sigTerm);
    signal(SIGTERM, sigTerm);
    Log->openLog(LOG_FILE::INTERNAL, "spacecrafter");
    window.initSDL();
    window.createWindow("Experiment", 600, 600, 0, 3, false, "~/.spacecrafter/data/icon.bpm");

    // varray->registerVertexBuffer(BufferType::SHAPE, BufferAccess::STATIC);

    auto vertices = std::vector<float>({
        0,0,0, 1,0,0, 1,1,0, 0,1,0,
        0,0,0, 0,1,0, 0,1,1, 0,0,1,
        0,0,0, 0,0,1, 1,0,1, 1,0,0,

        1,1,1, 0,1,1, 0,0,1, 1,0,1,
        1,1,1, 1,0,1, 1,0,0, 1,1,0,
        1,1,1, 1,1,0, 0,1,0, 0,1,1
    });

    auto colors = std::vector<float>({
        1,0,0, 1,0,0, 1,0,0, 1,0,0,
        0,1,0, 0,1,0, 0,1,0, 0,1,0,
        0,0,1, 0,0,1, 0,0,1, 0,0,1,

        0,1,1, 0,1,1, 0,1,1, 0,1,1,
        1,0,1, 1,0,1, 1,0,1, 1,0,1,
        1,1,0, 1,1,0, 1,1,0, 1,1,0,
    });

    std::cout << "1\n";
    auto shader = std::make_unique<shaderProgram>();
    shader->init("experiment.vert", "experiment.frag");

    std::cout << "2\n";
    auto mvp = Mat4f::scaling(0.7).translation(Vec3f(-0.5, -0.5, -0.5));
    const auto rotate = Mat4f::yawPitchRoll(0.05, 0.03, 0.04);

    std::cout << "3\n";
    auto varray = std::make_unique<VertexArray>();
    varray->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
    varray->registerVertexBuffer(BufferType::COLOR, BufferAccess::STATIC);

    varray->fillVertexBuffer(BufferType::POS3D, vertices);
    varray->fillVertexBuffer(BufferType::COLOR, colors);

    std::cout << "4\n";
    StateGL::disable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);
    StateGL::enable(GL_DEPTH_TEST);

    std::cout << "start\n";
    while (opened) {
        mvp = rotate * mvp;
        Renderer::clearColor();
        std::cout << "\ec";
        mvp.print();
        shader->use();
        shader->setUniform("Mat", mvp);
        Renderer::drawArrays(shader.get(), varray.get(), GL_QUADS, 0, 24);
        window.glSwapWindow();
    }

    Log->close();
    return 0;
}
