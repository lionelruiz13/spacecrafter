#include <GL/glew.h>
#include "mainModule/sdl_facade.hpp"
#include "tools/log.hpp"
#include "tools/stateGL.hpp"
#include "tools/OpenGL.hpp"

/*
#include "Window.hpp"

int main()
{
    Window window("Experiment", 800, 600);

    if (!window.isOpen)
        return 1;

    return 0;
}
/*/
int main()
{
    cLog *Log = cLog::get();
    SDLFacade window;

    Log->openLog(LOG_FILE::INTERNAL, "spacecrafter");
    window.initSDL();
    window.createWindow("Experiment", 800, 600, 0, 3, false, "~/.spacecrafter/data/icon.bpm");

    // auto varray = std::make_unique<VertexArray>();
    // varray->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
    // varray->registerVertexBuffer(BufferType::COLOR, BufferAccess::DYNAMIC);
    // varray->registerVertexBuffer(BufferType::SHAPE, BufferAccess::STATIC);

    float vertices[] = {
        0,0,0, 1,0,0, 1,1,0, 0,1,0,
        0,0,0, 0,1,0, 0,1,1, 0,0,1,
        0,0,0, 0,0,1, 1,0,1, 1,0,0,

        1,1,1, 0,1,1, 0,0,1, 1,0,1,
        1,1,1, 1,0,1, 1,0,0, 1,1,0,
        1,1,1, 1,1,0, 0,1,0, 0,1,1
    };
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);
    // varray->fillVertexBuffer(BufferType::POS3D, cube);
    SDL_Event event;

    bool opened = true;
    while (opened) {
        SDL_WaitEvent(&event);
        switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                opened = false;
                break;
            default:;
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_QUADS, 0, 72);
        window.glSwapWindow();
    }
    glDisableVertexAttribArray(0);

    Log->close();
    return 0;
}
//*/
