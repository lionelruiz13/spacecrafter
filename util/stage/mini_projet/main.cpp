#include <SDL2/SDL.h>
#include <iostream>
#include <assert.h>

int main()
{
    SDL_GLContext context;
    SDL_Window *window(0);
    SDL_Event event;
    bool opened = true;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Erreur lors de l'initialisation de la SDL : " << SDL_GetError() << std::endl;
        SDL_Quit();

        return 1;
    }

    window = SDL_CreateWindow("Experiment", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if (!window) {
        std::cerr << "Error: Faild to create window" << std::endl;
        SDL_Quit();

        return 1;
    }

    context = SDL_GL_CreateContext(window);

    if(context == 0) {
        std::cerr << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 1;
    }

    while (opened) {
        SDL_WaitEvent(&event);
        switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                opened = false;
                break;
            default:;
        }
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
