// Example program:
// Using SDL2 to create an application window

#include <SDL2/SDL.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "config.h"

int main(int argc, char* argv[])
{
    SDL_Window    *window = nullptr;
    SDL_Renderer  *renderer = nullptr;

	int IMAGE_WIDTH = 320;
	int IMAGE_HEIGHT = 240;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2
	bool isAlive = true;

	// test config.ini
	std::cout << "package      " << PACKAGE  << std::endl;
	std::cout << "version      " << VERSION << std::endl;
	std::cout << "app_name     " << APP_NAME << std::endl;
	std::cout << "app_lname    " << APP_LOWER_NAME << std::endl;
	std::cout << "user_name    " << USER_NAME << std::endl;
	std::cout << "user_edition " << USER_EDITION << std::endl;

    // Create an application window with the following settings:
    window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, IMAGE_WIDTH*2, IMAGE_HEIGHT*2, 0);
    if (window == nullptr) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		fprintf(stderr, "SDL_CreateRenderer Error\n");
		return 2;
	}

	while (isAlive) {
	    SDL_Event e;
	    while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) { // click close icon then quit
				isAlive = false;
			  }
			  if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE) // press ESC the quit
					isAlive = false;
			  }
	    }

	    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 10, 255, 10, SDL_ALPHA_OPAQUE);
        SDL_RenderDrawLine(renderer, 0, IMAGE_HEIGHT, IMAGE_WIDTH*2, IMAGE_HEIGHT);
        SDL_RenderDrawLine(renderer, IMAGE_WIDTH, 0, IMAGE_WIDTH, IMAGE_HEIGHT *2);
		SDL_RenderPresent(renderer);
	}
    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
    return 0;
}
