//~ #include "../../src/vecmath.hpp"

//~ #include "obj_parser.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <math.h>

#include "TargetCamera.hpp"
#include "FreeCamera.hpp"
#include "SDLFacade.hpp"

#define __main__
#include "shader.hpp"
#include "ojm.hpp"

// camera matrices. it's easier if they are global
glm::mat4 view_mat;
glm::mat4 proj_mat;
glm::mat4 normal_mat;
//~ glm::vec3 cam_pos(0.0f, 0.0f, 50.0f);
glm::vec3 ojmMesh_pos_wor(0.0f,0.0f,0.0f);

float dt = 1.f;
const float MOVE_SPEED = 0.5f;
  
const float MOUSE_FILTER_WEIGHT=0.75f;
const int MOUSE_HISTORY_BUFFER_SIZE = 10;
glm::vec2 mouseHistory[MOUSE_HISTORY_BUFFER_SIZE];

float mouseX=0, mouseY=0; //filtered mouse values
bool useFiltering = true;
int state = 0, oldX=0, oldY=0;
float rX=0, rY=0, fov = 45;


void filterMouseMoves(float dx, float dy) {
    for (int i = MOUSE_HISTORY_BUFFER_SIZE - 1; i > 0; --i) {
        mouseHistory[i] = mouseHistory[i - 1];
    }

    // Store current mouse entry at front of array.
    mouseHistory[0] = glm::vec2(dx, dy);

    float averageX = 0.0f;
    float averageY = 0.0f;
    float averageTotal = 0.0f;
    float currentWeight = 1.0f;

    // Filter the mouse.
    for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE; ++i)
    {
		glm::vec2 tmp=mouseHistory[i];
        averageX += tmp[0] * currentWeight;
        averageY += tmp[1] * currentWeight;
        averageTotal += 1.0f * currentWeight;
        currentWeight *= MOUSE_FILTER_WEIGHT;
    }

    mouseX = averageX / averageTotal;
    mouseY = averageY / averageTotal;
	
}



int gl_width = 1920;
int gl_height = 1080;

std::string programName = "OJM_VIEWER";



int main (int argc, char **argv)
{
	SDLFacade* mainSDL = nullptr;
	CFreeCamera cam;
	std::string fileToLoad;
/*--------------------------------START OPENGL--------------------------------*/

	mainSDL = new SDLFacade(gl_width, gl_height);
	mainSDL->init(programName);

	float lum = 0.5;

/*------------------------------CREATE GEOMETRY-------------------------------*/
	if (argc!=2) {
		printf("%s need Argument\n", argv[0]);
		printf("Exemple: %s <file.ojm>\n", argv[0]);
		fileToLoad = "Voyager.ojm";
	} else
		fileToLoad = argv[1];

	Ojm ojmMesh(fileToLoad,"", 10.0f);

	if (!ojmMesh.getOk()) {
		printf("ojm not ok\n");
		return -2;
	}

/*-------------------------------CREATE SHADERS-------------------------------*/
	shaderProgram* shaderOJM;
	shaderOJM= new shaderProgram();
	shaderOJM->init("shaderOJM.vert", "", "", "","shaderOJM.frag");
	shaderOJM->setUniformLocation("ModelViewMatrix");
	shaderOJM->setUniformLocation("NormalMatrix");
	shaderOJM->setUniformLocation("ProjectionMatrix");
	shaderOJM->setUniformLocation("MVP");
	shaderOJM->setUniformLocation("Light.Position");
	shaderOJM->setUniformLocation("Light.Intensity");
	shaderOJM->setUniformLocation("Material.Ka");
	shaderOJM->setUniformLocation("Material.Kd");
	shaderOJM->setUniformLocation("Material.Ks");
	shaderOJM->setUniformLocation("Material.Ns");
	shaderOJM->setUniformLocation("useTexture");


/*---------------------------SET RENDERING DEFAULTS---------------------------*/

	glm::mat4 model_mat = glm::translate(glm::mat4(1), ojmMesh_pos_wor) * glm::rotate(glm::mat4(1),90.0f, glm::vec3(1,0,0));

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glEnable(GL_CULL_FACE); // enable cull_face
	glFrontFace(GL_CW);
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5, 0.5, 0.5, 1.0); // grey background to help spot mistakes
	glViewport(0, 0, gl_width, gl_height);

	cam.SetupProjection(45, (GLfloat)gl_width/gl_height);	
	glm::vec3 p = glm::vec3(10,0,0);
	cam.SetPosition(p);
	glm::vec3 look =  glm::normalize(p);
	
	//rotate the camera for proper orientation
	float yaw = glm::degrees(float(atan2(look.z, look.x)+M_PI));
	float pitch = glm::degrees(asin(look.y));
	rX = yaw;
	rY = pitch;
	if(useFiltering) {
		for (int i = 0; i < MOUSE_HISTORY_BUFFER_SIZE ; ++i) {
			mouseHistory[i] = glm::vec2(rX, rY);
		}
	}
	cam.Rotate(rX,rY,0);

/*-------------------------------RENDERING LOOP-------------------------------*/
	bool loop = true;
	SDL_Event event;

	// control keys
	bool cam_moved = false;
	//~ glm::vec3 move (0.0, 0.0, 0.0);
	//~ float cam_yaw = 0.0f; // y-rotation in degrees
	//~ float cam_pitch = 0.0f;
	//~ float cam_roll = 0.0;

	// update timers
	unsigned int previous_time= SDL_GetTicks();;
	unsigned int current_time = previous_time;
	unsigned int elapsed_time = 0;
	double elapsed_seconds;

	//std::cout << "calGetPos  " << glm::to_string(cam.GetPosition() ) << std::endl;
	//std::cout << "calGetProj " << glm::to_string(cam.GetProjectionMatrix() ) << std::endl;
	//std::cout << "model_mat  " << glm::to_string( model_mat ) << std::endl;

	while (loop) 
	{
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

		// control keys
		cam_moved = false;
		//~ move=glm::vec3(0.0, 0.0, 0.0);
		//~ cam_yaw = 0.0f; // y-rotation in degrees
		//~ cam_pitch = 0.0f;
		//~ cam_roll = 0.0;

		float dx=0, dy=0;
		// update other events like input handling 
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				loop = false;

			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_ESCAPE:
					loop = false;
					break;

				//moving cam
				case SDLK_z:
					cam.Walk(dt);
					cam_moved = true;
					break;
				case SDLK_x:
					cam.Walk(-dt);
					cam_moved = true;
					break;

				case SDLK_q:
					cam.Strafe(-dt);
					cam_moved = true;
					break;
				case SDLK_d:
					cam.Strafe(dt);
					cam_moved = true;
					break;

				case SDLK_a:
					cam.Lift(-dt);
					cam_moved = true;
					break;
				case SDLK_e:
					cam.Lift(dt);
					cam_moved = true;
					break;

				case SDLK_t:
					lum +=0.05;
					std::cout << "Light.intensity: " << lum << std::endl;
					break;
				case SDLK_g:
					lum -=0.05;
					std::cout << "Light.intensity: " << lum << std::endl;
					break;

				// yaw
				case SDLK_LEFT:
					cam.Rotate(1.0,0.0,0.0);
					cam_moved = true;
					break;
				case SDLK_RIGHT:
					cam.Rotate(-1.0,0.0,0.0);
					cam_moved = true;
					break;

				//pitch
				case SDLK_UP:
					cam.Rotate(0.0,1.0,0.0);
					cam_moved = true;
					break;


				case SDLK_DOWN:
					cam.Rotate(0.0,-1.0,0.0);
					cam_moved = true;
					break;

				// roll
				case SDLK_w:
					cam.Rotate(0.0,0.0,1.0);
					cam_moved = true;
					break;

				case SDLK_c:
					cam.Rotate(0.0,0.0,-1.0);
					cam_moved = true;
					break;

				default:
					break;
				}
			}

			if (event.type == SDL_MOUSEMOTION)
			{
				int x = event.motion.x;
				int y = event.motion.y;
				rY += (y - oldY)/5.0f;
				rX += (oldX-x)/5.0f;
				if(useFiltering)
					filterMouseMoves(rX, rY);
				else {
					mouseX = rX;
					mouseY = rY;
				}
				cam.Rotate(-mouseX,mouseY, 0);
				oldX = x;
				oldY = y;
			}

		

		}

		//~ if (cam_moved)
			//~ cam.Move(dx, dy);
		cam.Update();

		view_mat = cam.GetViewMatrix();
		//std::cout << "view_mat " << glm::to_string( view_mat ) << std::endl;
		proj_mat = cam.GetProjectionMatrix();
		//std::cout << "proj_mat " << glm::to_string( proj_mat ) << std::endl;

		normal_mat = glm::transpose(glm::inverse(view_mat * model_mat));
		//std::cout << "normal_mat " << glm::to_string( normal_mat ) << std::endl;

		shaderOJM->use();
		shaderOJM->setUniform("ModelViewMatrix" , view_mat * model_mat);
		shaderOJM->setUniform("ProjectionMatrix" , proj_mat);
		shaderOJM->setUniform("NormalMatrix" , normal_mat);
		shaderOJM->setUniform("MVP" , proj_mat * view_mat * model_mat);

		shaderOJM->setUniform("Light.Position" , view_mat *  glm::vec4(0.0, 0.0, 0.0, 1.0));
		shaderOJM->setUniform("Light.Intensity" , lum*glm::vec3(1.0, 1.0, 1.0));

		ojmMesh.draw(shaderOJM);
		glUseProgram(0);

		// update timers
		current_time = SDL_GetTicks();
		elapsed_time = current_time - previous_time;
		elapsed_seconds = 1.0*elapsed_time/1000.0;
		previous_time = current_time;
		if (elapsed_time<15)
			SDL_Delay(10);

		mainSDL-> swapWindow();
	}

	// close GL context and any other GLFW resources
	mainSDL -> cleanup();

	return 0;
}
