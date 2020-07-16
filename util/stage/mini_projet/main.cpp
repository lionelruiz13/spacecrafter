#include <GL/glew.h>
#include "mainModule/sdl_facade.hpp"
#include "tools/log.hpp"

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
    window.createWindow(800, 600, 0, 3, false, "~/.spacecrafter/data/icon.bpm");

    //*
    const float vertices[] = {-0.5, -0.5, 0, 0.5, 0.5, -0.5,
                              -0.8, -0.8, -0.3, -0.8, -0.8, -0.3}; //*/
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

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
        glDrawArrays(GL_TRIANGLES, 0, 6);
        window.glSwapWindow();
    }
    glDisableVertexAttribArray(0);

    Log->close();
    return 0;
}
//*/
