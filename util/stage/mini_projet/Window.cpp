#include "Window.hpp"

char Window::instances = 0;

Window::Window(const char *title, int width, int heigh) : isOpen(opened), window(0), context(0)
{
    if (instances++ == 0) {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "Erreur lors de l'initialisation de la SDL : " << SDL_GetError() << std::endl;
            SDL_Quit();
            return;
        }

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    }

    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, heigh, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if (!window) {
        std::cerr << "Error: Faild to create window" << std::endl;

        return;
    }

    context = SDL_GL_CreateContext(window);

    if(context == 0) {
        std::cerr << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);

        return;
    }

    if (instances == 1) {
        GLenum initGLEW(glewInit());
        switch (initGLEW) {
            case GLEW_OK:
                break;
            default:
                std::cerr << "Error while initializing GLEW : " << glewGetErrorString(initGLEW) << std::endl;
                SDL_GL_DeleteContext(context);
                SDL_DestroyWindow(window);
                return;
        }
    }
    opened = true;
}

Window::~Window()
{
    if (opened) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
    }
    if (--instances == 0)
        SDL_Quit();
}

void Window::close()
{
    if (opened) {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        opened = false;
    }
}
