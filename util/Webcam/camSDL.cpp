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
    SDL_Texture   *texCam1 = nullptr;
    SDL_Rect      rectCam1;
    SDL_Texture   *texCam2 = nullptr;
    SDL_Rect      rectCam2;
    SDL_Texture   *texCam3 = nullptr;
    SDL_Rect      rectCam3;

	int IMAGE_WIDTH = 640;
	int IMAGE_HEIGHT = 480;

    SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2
	bool isAlive = true;

    Webcam webcam0("/dev/video0", IMAGE_WIDTH, IMAGE_HEIGHT);
    //~ Webcam webcam1("/dev/video1", IMAGE_WIDTH, IMAGE_HEIGHT);

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

	texCam0 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, IMAGE_WIDTH, IMAGE_HEIGHT);
	rectCam0.w = IMAGE_WIDTH;
	rectCam0.h = IMAGE_HEIGHT;
	rectCam0.x = 0;
	rectCam0.y = 0;
	texCam1 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, IMAGE_WIDTH, IMAGE_HEIGHT);
	rectCam1.w = IMAGE_WIDTH;
	rectCam1.h = IMAGE_HEIGHT;
	rectCam1.x = IMAGE_WIDTH;
	rectCam1.y = 0;


	texCam2 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, IMAGE_WIDTH, IMAGE_HEIGHT);
	rectCam2.w = IMAGE_WIDTH;
	rectCam2.h = IMAGE_HEIGHT;
	rectCam2.x = 0;
	rectCam2.y = IMAGE_HEIGHT;
	texCam3 = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, IMAGE_WIDTH, IMAGE_HEIGHT);
	rectCam3.w = IMAGE_WIDTH;
	rectCam3.h = IMAGE_HEIGHT;
	rectCam3.x = IMAGE_WIDTH;
	rectCam3.y = IMAGE_HEIGHT;


    auto frame0 = webcam0.frame();
    //~ auto frame1 = webcam1.frame();
    
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
			  }
	    }

		frame0 = webcam0.frame();
		//~ frame1 = webcam1.frame();

		SDL_UpdateTexture(texCam0, &rectCam0, frame0.data, IMAGE_WIDTH*3 );
		//~ SDL_UpdateTexture(texCam1, &rectCam0, frame1.data, IMAGE_WIDTH*3 );
		SDL_UpdateTexture(texCam1, &rectCam0, frame0.data, IMAGE_WIDTH*3 );
		SDL_UpdateTexture(texCam2, &rectCam0, frame0.data, IMAGE_WIDTH*3 );
		SDL_UpdateTexture(texCam3, &rectCam0, frame0.data, IMAGE_WIDTH*3 );
		
		SDL_SetTextureColorMod(texCam1, 255, 0, 0);
		SDL_SetTextureColorMod(texCam2, 0, 255, 0);
		SDL_SetTextureColorMod(texCam3, 0, 0, 255);
		
		//~ SDL_UpdateTexture(texCam2, &rectCam0, frame0.data, IMAGE_WIDTH*3 );
		//~ SDL_UpdateTexture(texCam3, &rectCam0, frame1.data, IMAGE_WIDTH*3 );
		//~ SDL_SetTextureColorMod(texCam2, 0, 255, 0);
		//~ SDL_SetTextureColorMod(texCam3, 255, 0, 0);

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texCam0, NULL, &rectCam0);
		SDL_RenderCopy(renderer, texCam1, NULL, &rectCam1);
		SDL_RenderCopy(renderer, texCam2, NULL, &rectCam2);
		SDL_RenderCopy(renderer, texCam3, NULL, &rectCam3);
		
		//~ SDL_RenderCopyEx(renderer,texCam0, NULL, &rectCam2, 0, NULL, SDL_FLIP_VERTICAL);
		//~ SDL_RenderCopyEx(renderer,texCam1, NULL, &rectCam3, 0, NULL, SDL_FLIP_VERTICAL);

		//~ SDL_RenderCopy(renderer, texCam2, NULL, &rectCam2);
		//~ SDL_RenderCopy(renderer, texCam3, NULL, &rectCam3);

        SDL_RenderDrawLine(renderer, 0, IMAGE_HEIGHT, IMAGE_WIDTH*2, IMAGE_HEIGHT);
        SDL_RenderDrawLine(renderer, IMAGE_WIDTH, 0, IMAGE_WIDTH, IMAGE_HEIGHT *2);

		SDL_RenderPresent(renderer);
	}


    SDL_DestroyWindow(window);

    // Clean up
    SDL_Quit();
    return 0;
}
