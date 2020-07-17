#include <GL/glew.h>
#include "mainModule/sdl_facade.hpp"
#include "tools/log.hpp"
#include "tools/stateGL.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"
#include "tools/Renderer.hpp"
#include "signal.h"

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
    Log->openLog(LOG_FILE::INTERNAL, "spacecrafter");
    window.initSDL();
    window.createWindow("Experiment", 800, 600, 0, 3, false, "~/.spacecrafter/data/icon.bpm");

    // varray->registerVertexBuffer(BufferType::SHAPE, BufferAccess::STATIC);

    auto vertices = std::vector<float>({
        0,0,0, 1,0,0, 1,1,0, 0,1,0,
        0,0,0, 0,1,0, 0,1,1, 0,0,1,
        0,0,0, 0,0,1, 1,0,1, 1,0,0,

        1,1,1, 0,1,1, 0,0,1, 1,0,1,
        1,1,1, 1,0,1, 1,0,0, 1,1,0,
        1,1,1, 1,1,0, 0,1,0, 0,1,1
    });

    std::cout << "1\n";
    auto shader = std::make_unique<shaderProgram>();
    shader->init("experiment.vert", "experiment.frag");
    shader->unuse();

    std::cout << "2\n";
    auto varray = std::make_unique<VertexArray>();
    varray->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);

    varray->fillVertexBuffer(BufferType::POS3D, vertices);

    std::cout << "3\n";
    StateGL::disable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);

    std::cout << "start\n";
    while (opened) {
        Renderer::clearColor();
        Renderer::drawArrays(shader.get(), varray.get(), GL_QUADS, 0, 24);
        window.glSwapWindow();
    }

    Log->close();
    return 0;
}
