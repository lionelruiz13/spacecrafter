#ifndef _WINDOW_HPP_
#define _WINDOW_HPP_

#define GL3_PROTOTYPES 1
#include <GL/glew.h>

#include <SDL2/SDL.h>
#include <iostream>
#include <assert.h>

class Window
{
public:
    Window(const char *title, int width, int heigh);
    ~Window();
    void close();
    void swap() {SDL_GL_SwapWindow(window);}
    const bool &isOpen;
private:
    SDL_Window *window;
    SDL_GLContext context;
    bool opened = false;
    static char instances;
};

#endif /* end of include guard: _WINDOW_HPP_ */
