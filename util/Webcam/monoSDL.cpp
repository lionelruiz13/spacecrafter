// Example program:
// Using SDL2 to create an application window

#include <SDL2/SDL.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "webcam.h"



int main(int argc, char* argv[])
{
    SDL_Window    *window = nullptr;
    SDL_Renderer  *renderer = nullptr;
    SDL_Texture   *texCam0 = nullptr;
    SDL_Rect      rectCam0;

	int IMAGE_WIDTH = 640;
	int IMAGE_HEIGHT = 480;

	int contrast = 100;
	int brightness = 100;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2
	bool isAlive = true;

    Webcam webcam0("/dev/video0", IMAGE_WIDTH, IMAGE_HEIGHT);

    // Create an application window with the following settings:
    window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, IMAGE_WIDTH, IMAGE_HEIGHT, 0);
    if (window == nullptr) {
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == nullptr) {
		fprintf(stderr, "SDL_CreateRenderer Error\n");
		return 2;
	  }

	texCam0 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, IMAGE_WIDTH, IMAGE_HEIGHT);
	rectCam0.w = IMAGE_WIDTH;
	rectCam0.h = IMAGE_HEIGHT;
	rectCam0.x = 0;
	rectCam0.y = 0;

    auto frame0 = webcam0.frame();
    
    std::ofstream image;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);

	while (isAlive) {
	    SDL_Event e;
	    while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT) { // click close icon then quit
				isAlive = false;
			  }
			  if (e.type == SDL_KEYDOWN) {
				if (e.key.keysym.sym == SDLK_ESCAPE) // press ESC the quit
					isAlive = false;

				//~ if (e.key.keysym.sym == SDLK_c) {
					//~ contrast += 5;
					//~ webcam0.setContrast(contrast);
					//~ printf("modification contast : %i\n", contrast);
				//~ }
				//~ if (e.key.keysym.sym == SDLK_d) {
					//~ contrast -= 5;
					//~ webcam0.setContrast(contrast);
					//~ printf("modification contast : %i\n", contrast);
				//~ }

				//~ if (e.key.keysym.sym == SDLK_b) {
					//~ brightness += 5;
					//~ webcam0.setContrast(brightness);
					//~ printf("modification brightness : %i\n", brightness);
				//~ }
				//~ if (e.key.keysym.sym == SDLK_g) {
					//~ brightness -= 5;
					//~ webcam0.setBrightness(brightness);
					//~ printf("modification brightness : %i\n", brightness);
				//~ }


			  }
	    }

		frame0 = webcam0.frame();

		SDL_UpdateTexture(texCam0, &rectCam0, frame0.data, IMAGE_WIDTH*3 );
		SDL_SetTextureColorMod(texCam0, 255, 0, 0);
		
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texCam0, NULL, &rectCam0);
		SDL_RenderPresent(renderer);
	}


    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
    return 0;
}
