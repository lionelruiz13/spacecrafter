#ifndef MAIN_HPP_INCLUDED
#define MAIN_HPP_INCLUDED

#include <iostream>
#include <string>
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>//va_list va_start va_arg va_end
#include <assert.h>
#include <sstream>
#include <climits>
#include <vector>

#include <GL/glew.h>

#include <SDL2/SDL.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>


#ifndef IMG_Load /// ou inclure SDL image
#define IMG_Load SDL_LoadBMP
#endif // IMG_Load

#ifdef _WIN32
#include <windows.h>
#else
#define APIENTRY
#endif

#define PI (0x3.243F6A8885p0l)
#define G ((double)9.81)
#define BOOL unsigned char
#define Uchar  unsigned char
#define Ushort unsigned short
#define Uint   unsigned int

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
#define rmask  0xff000000
#define gmask  0x00ff0000
#define bmask  0x0000ff00
#define amask  0x000000ff
#else

#define rmask  0x000000ff
#define gmask  0x0000ff00
#define bmask  0x00ff0000
#define amask  0xff000000
#endif

#define FORWARD     0x0
#define BACK        0x1
#define RIGHT       0x2
#define LEFT        0x3
#define WANTTOJUMP  0x4
#define DOWN        0x5



struct Data {
	Uint32 SCREENX  ;
	Uint32 SCREENY  ;
	Uint32 SCREENX2 ;
	Uint32 SCREENY2 ;
	double ASPECT   ;
	Uint8  bpp      ;

	SDL_Window *window;
	SDL_GLContext context;
};


struct Time {
	Uint time;
	double fps;

	Uint fpscounter;
	Time() {
		time=SDL_GetTicks();
		fps=150.0;
		fpscounter=0;
	}
	void update() { // compute time
		if(SDL_GetTicks()-time>1000) {
			fps = fpscounter;

			time=SDL_GetTicks();
			fpscounter=0;
		}
		fpscounter++;
	}
};

#endif // MAIN_HPP_INCLUDED


