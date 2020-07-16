#include "Window.hpp"

int main()
{
    Window window("Experiment", 800, 600);

    if (!window.isOpen)
        return 1;

    const float vertices[] = {-0.5, -0.5, 0, 0.5, 0.5, -0.5,
                              -0.8, -0.8, -0.3, -0.8, -0.8, -0.3};

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

    SDL_Event event;

    while (window.isOpen) {
        SDL_WaitEvent(&event);
        switch (event.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                window.close();
                break;
            default:;
        }
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        window.swap();
    }
    glDisableVertexAttribArray(0);

    return 0;
}
