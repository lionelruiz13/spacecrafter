#include "main.hpp"
#include "vecmath.hpp"
#include "Camera.hpp"
#include "Planet.hpp"
#include "atmosphere_ext.hpp"

// ATMOSPHERE_RADIUS rayon pure de l'atmosphere
#define EARTH_RADIUS 1.0
#define ATM_RADIUS   1.1
// #define EARTH_RADIUS 0.0000433
// #define ATM_RADIUS   0.0000436

int main(int argc,char**argv)
{
	Data*data=new Data;
	SDL_Surface*icon;
	Uint8 windowIsOpen=true;
	SDL_Event event;


	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);

	/* Données de base */
	SDL_DisplayMode dm;
	if (SDL_GetDesktopDisplayMode(0, &dm) != 0) {
		SDL_Log("SDL_GetDesktopDisplayMode failed: %s", SDL_GetError());
		exit(EXIT_FAILURE);
	}
	data->SCREENX  =dm.w;
	data->SCREENY  =dm.h;
	data->SCREENX2 =data->SCREENX/2;
	data->SCREENY2 =data->SCREENY/2;
	data->ASPECT   =(double)data->SCREENX/data->SCREENY;
	data->bpp      =32;

	data->window=SDL_CreateWindow("Earth representation by Jerome Lartillot", 50,50,1200,900,SDL_WINDOW_OPENGL|SDL_WINDOW_SHOWN);
	data->SCREENX  =1200;
	data->SCREENY  =900;
	data->SCREENX2 =data->SCREENX/2;
	data->SCREENY2 =data->SCREENY/2;
	data->ASPECT   =(double)data->SCREENX/data->SCREENY;


	data->context=SDL_GL_CreateContext(data->window);

	/* initialisation de GLEW */
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		// Problem: glewInit failed, something is seriously wrong.
		std::cout<<"Error: "<< glewGetErrorString(err)<<std::endl;
	}
	std::cout<<"Status: Using GLEW "<< glewGetString(GLEW_VERSION)<<std::endl;

	/* Icône */
	icon=SDL_CreateRGBSurface(0,16,16,32,0,0,0,0);
	SDL_FillRect(icon,NULL,SDL_MapRGB(icon->format,0,255,0));
	SDL_SetWindowIcon(data->window,icon);

	SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
	glEnable            (GL_DEPTH_TEST);
	// glTexEnvf           (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_REPLACE);
	glMatrixMode(GL_MODELVIEW); // ou GL_PROJECTION
	SDL_ShowCursor(SDL_DISABLE);
	if(!SDL_SetRelativeMouseMode(SDL_TRUE))std::cout<<"Error: couldn't set relative mouse mode!"<<std::endl;
	glActiveTexture(GL_TEXTURE0);

	SiriusCamera camera("../config.txt");


	std::vector<std::vector<Mat4f> > matrixArray(4);

	matrixArray[0].push_back(Mat4f::identity()); // mercury
	matrixArray[1].push_back(Mat4f::identity()); // venus
	matrixArray[2].push_back(Mat4f::identity()); // earth
	matrixArray[3].push_back(Mat4f::identity()); // mars

	Mat4f moonRelativeMatrix=Mat4f::identity();
	matrixArray[2].push_back(Mat4f::identity()); // moon

	matrixArray[0][0].r[12]=40; // mercury
	matrixArray[1][0].r[12]=80; // venus
	matrixArray[2][0].r[12]=140; // earth
	matrixArray[3][0].r[12]=200; // mars

	moonRelativeMatrix.r[12]=20;


	camera.init(&matrixArray,2);

	float rotationSpeed=0.05;
	Vec3f up(0,0,1);
	Mat4f view;
	Mat4f projection= Mat4f::perspective(73,data->ASPECT,0.01,450);
	Vec2f resolution(data->SCREENX,data->SCREENY);

	std::cout<<"-------------- defaultProgram ---------------"<<std::endl;

	shaderProgram defaultProgram;
	defaultProgram.init("../data/shaders/default.vert","","","","../data/shaders/default.frag");
	defaultProgram.setUniformLocation("texture0");


	PlanetData settings;

	// good settings
	settings.texturePath   = "../data/textures/earth_texture.png";
	settings.heightmapPath = "../data/textures/earth_heightmap.png";
	settings.normalsPath   = "../data/textures/earth_normal.png";

	settings.textureUnit   = 1;
	settings.heightmapUnit = 2;
	settings.normalsUnit   = 3;
	std::cout<<"-------------- LOADING EARTH ---------------"<<std::endl;
	Planet planet(settings);

	AtmosphereExt atmosphere;


	// std::cout<<"-------------- LOADING SUN ---------------"<<std::endl;
	// GPU::Model3D sun;
	// sun.load_OBJ("../data/models/sphere32x16.obj","../data/textures/sun_wrong_texture.bmp");


	int texture_units;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &texture_units);
	std::cout<<"Max textures: "<<texture_units<<std::endl;

	// On crée le temps en dernier pour qu'il soit initialisé après les chargements...
	Time time;

	while(windowIsOpen) {
		// EVENTS
		while(SDL_PollEvent(&event)) {
			if(event.type==SDL_QUIT || event.type==SDL_KEYDOWN&& event.key.keysym.sym==SDLK_ESCAPE)
				windowIsOpen=false;

			camera.listen(event);

			switch(event.type) {
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym) {
						case SDLK_1:
							rotationSpeed=0.0;
							break;
						case SDLK_2:
							rotationSpeed=0.02;
							break;
						case SDLK_3:
							rotationSpeed=0.05;
							break;
						case SDLK_4:
							rotationSpeed=0.1;
							break;
						case SDLK_5:
							rotationSpeed=0.15;
							break;
						case SDLK_6:
							rotationSpeed=0.30;
							break;
						case SDLK_7:
							rotationSpeed=0.45;
							break;
						case SDLK_8:
							rotationSpeed=0.8;
							break;
						case SDLK_9:
							rotationSpeed=1.0;
							break;
					}
					break;
			}
		}

		// UPDATES
		time.update();
		// glUseProgram(0);

		// earth
		matrixArray[2][0] = Mat4f::rotation(up,M_PI*0.25*rotationSpeed/time.fps)*
		                    matrixArray[2][0]*
		                    Mat4f::rotation(up,M_PI*0.25*rotationSpeed/time.fps);

		matrixArray[2][1] = matrixArray[2][0]*moonRelativeMatrix;

		camera.update();


		// DRAWS
		// ViewBeforeLookAt :
		//matrix.hpp il s'agit de retrouver m dans la fonction lookAtFromMatrix qui donne le résultat view
		// donc tu as view et tu dois trovuer la matrice m de la fonction ci dessus.
		//
		// ViewBeforeLookAt sert pour aller a droite gauche haut bas ... dans la réalité (pour placer ta camera)
		// mais comme le repère opengl n'est pas le repère de la réalité, il faut transformer cette matrice pour obtenir
		// la matrice view (pour le monde opengl)
	

		glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
		glClearColor(0,0,0,0);
		glLoadIdentity();
		gluPerspective(73,data->ASPECT,0.01,500);
		camera.lookAt();
		view=camera.getViewMatrix();




		// defaultProgram.use();

		// defaultProgram.setUniform("texture0",0);

		// sun.bindBuffersAndDraw(GL_TRIANGLES);

		// glDisable(GL_TEXTURE_2D);
		// glBegin(GL_LINES);
		// glColor3f(0,0,1);
		// glVertex3d(-8,0,0);
		// glVertex3d(8,0,0);
		// glColor3f(0,1,0);
		// glVertex3d(0,-8,0);
		// glVertex3d(0,8,0);
		// glColor3f(1,0,0);
		// glVertex3d(0,0,-8);
		// glVertex3d(0,0,8);
		// glEnd();


		// matrixArray[2][0] = matrice modèle de la terre
		// pour obtenir la matrice modèle de l'atmosphère:
		// translation(positionTerre) * scaling(ATM_RADIUS) ou la matrice model de la terre * scaling (atmosphere)
		Mat4f view = camera.getViewMatrix();

		/*
		*
		*	GESTION DE LA PLANETE 
		*
		*/

		planet.useProgram();
		glEnable(GL_CULL_FACE); // enabling face culling to optimize
		glCullFace(GL_BACK);

		planet.setVp(projection * view);
		planet.setModel(matrixArray[2][0]);
		//affiche la planete...
		planet.drawWithTessellation(matrixArray[2][0],view,projection,resolution);

		glDisable(GL_CULL_FACE);
		glUseProgram(0);

		/*
		*
		*	GESTION DE L'ATMOSPHERE 
		*
		*/

		atmosphere.use();

		atmosphere.setViewBeforeLookAt(camera.getViewBeforeLookAtMatrix());
		atmosphere.setView(view);
		atmosphere.setVp(projection * view);


		atmosphere.setModel(matrixArray[2][0]* Mat4f::scaling(Vec3f(ATM_RADIUS,ATM_RADIUS,ATM_RADIUS)));
		atmosphere.setPlanetRadius(EARTH_RADIUS);
		atmosphere.setAtmRadius(ATM_RADIUS);
		atmosphere.setAtmAlphaScale(0.5);

		atmosphere.draw();

		glUseProgram(0);


		glFlush();
		SDL_GL_SwapWindow(data->window);
		SDL_Delay(10);
	}

	/* Destruction de la fenêtre et du contexte */
	SDL_GL_DeleteContext(data->context);
	SDL_DestroyWindow(data->window);

	SDL_Quit();
	delete data;
	return 0;
}
