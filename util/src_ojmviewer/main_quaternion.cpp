/*
|******************************************************************************|
| Virtual Camera with Quaternions.                                             |
| quaternion code is at the top of this file.                                  |
|                                                                              |
| controls:                                                                    |
| pitch = up,down arrow keys                                                   |
| yaw = left,right arrow keys                                                  |
| roll = z,c keys                                                              |
| move forward/back = w,s keys                                                 |
| move left/right = a,d keys                                                   |
|                                                                              |
\******************************************************************************/


#include "../../src/vecmath.hpp"
#include "quaternion.hpp"

//~ #include "obj_parser.h"
#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <iostream>

#include <math.h>

#define __main__
#include "../../src/log.hpp"
#include "../../src/shader.hpp"
#include "../../src/ojm.hpp"

// camera matrices. it's easier if they are global
Mat4f view_mat;
Mat4f proj_mat;
Mat4f normal_mat;
Vec3f cam_pos(0.0f, 0.0f, 50.0f);
Vec3f ojmMesh_pos_wor(0.0f,0.0f,-50.0f);

int gl_width = 1024;
int gl_height = 900;

std::string programName = "OJM_VIEWER";

class SDLFacade
{
public:
	SDLFacade(int _width, int _height);
	~SDLFacade();
	bool init(const std::string &programName);
	void swapWindow();
	void cleanup();
private:
	bool SetOpenGLAttributes();
	void PrintSDL_GL_Attributes();
	void CheckSDLError(int line);
	SDL_Window *mainWindow=nullptr;
	SDL_GLContext mainContext;
	int width, height;
};

SDLFacade::SDLFacade(int _width, int _height)
{
	width = _width;
	height = _height;
}

SDLFacade::~SDLFacade()
{}


bool SDLFacade::init(const std::string &programName)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cout << "Failed to init SDL\n";
		return false;
	}
	mainWindow = SDL_CreateWindow(programName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,gl_width, gl_height, SDL_WINDOW_OPENGL);
	if (!mainWindow) {
		std::cout << "Unable to create window\n";
		CheckSDLError(__LINE__);
		return false;
	}
	mainContext = SDL_GL_CreateContext(mainWindow);
	SetOpenGLAttributes();
	PrintSDL_GL_Attributes();
	SDL_GL_SetSwapInterval(1);

	glewInit();
	return true;
}

bool SDLFacade::SetOpenGLAttributes()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	return true;
}


void SDLFacade::cleanup()
{
	SDL_GL_DeleteContext(mainContext);
	SDL_DestroyWindow(mainWindow);
	SDL_Quit();
}

void SDLFacade::CheckSDLError(int line = -1)
{
	std::string error = SDL_GetError();
	if (error != ""){
		std::cout << "SLD Error : " << error << std::endl;
		if (line != -1)
			std::cout << "\nLine : " << line << std::endl;
		SDL_ClearError();
	}
}

void SDLFacade::PrintSDL_GL_Attributes()
{
	int value = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MAJOR_VERSION : " << value << std::endl;

	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
	std::cout << "SDL_GL_CONTEXT_MINOR_VERSION: " << value << std::endl;
}

void SDLFacade::swapWindow()
{
	SDL_GL_SwapWindow(mainWindow);
}


int main (int argc, char **argv)
{
	SDLFacade* mainSDL = nullptr;
/*--------------------------------START OPENGL--------------------------------*/

	mainSDL = new SDLFacade(gl_width, gl_height);
	mainSDL->init(programName);

	Log.open("prog.log", "script.log", "openGL.log", "tcp.log");

	Log.write("test !");

	float lum = 0.5;

/*------------------------------CREATE GEOMETRY-------------------------------*/
	if (argc!=2) {
		printf("%s need Argument\n", argv[0]);
		printf("Exemple: %s <file.ojm>\n", argv[0]);
		return -1;
	}
	Ojm ojmMesh(argv[1],"", 10.0f);

	//~ Ojm ojmMesh("Apollo11.ojm","",0.01f);
	//~ Ojm ojmMesh("cube_color.ojm","",1.0f);
	//~ Ojm ojmMesh("lro.ojm","",10.0f);

	if (!ojmMesh.getOk()) {
		printf("ojm not ok\n");
		return -2;
	}

	shaderProgram::setShaderDir("/usr/local/share/spacecrafter/shaders/");

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
	//~ shaderOJM->setUniformLocation("T");
	shaderOJM->printInformations();


/*-------------------------------CREATE CAMERA--------------------------------*/
	#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444
	// input variables
	float near = 0.1f; // clipping plane
	float far = 2000.1f; // clipping plane
	float fovy = 67.0f; // 67 degrees
	float aspect = (float)gl_width / (float)gl_height; // aspect ratio
	proj_mat = Mat4f::perspective(fovy, aspect, near, far);

	float cam_speed = 50.0f; // 1 unit per second
	float cam_heading_speed = 200.0f; // 30 degrees per second
	float cam_heading = 0.0f; // y-rotation in degrees
	Mat4f T = Mat4f::translation(Vec3f(-cam_pos.v[0], -cam_pos.v[1], -cam_pos.v[2]));
	// rotation matrix from my maths library. just holds 16 floats
	Mat4f R;
	// make a quaternion representing negated initial camera orientation
	float quaternion[4];
	create_versor(quaternion, -cam_heading, 0.0f, 1.0f, 0.0f);
	// convert the quaternion to a rotation matrix (just an array of 16 floats)
	quat_to_mat4(R.r, quaternion);
	// combine the inverse rotation and transformation to make a view matrix
	view_mat = R * T;
	// keep track of some useful vectors that can be used for keyboard movement
	Vec4f fwd(0.0f, 0.0f, -1.0f, 0.0f);
	Vec4f rgt(1.0f, 0.0f, 0.0f, 0.0f);
	Vec4f up(0.0f, 1.0f, 0.0f, 0.0f);

/*---------------------------SET RENDERING DEFAULTS---------------------------*/

	Mat4f model_mat = Mat4f::translation(ojmMesh_pos_wor) * Mat4f::xrotation(90.0*C_PI/180);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glEnable(GL_CULL_FACE); // enable cull_face
	glFrontFace(GL_CW);
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5, 0.5, 0.5, 1.0); // grey background to help spot mistakes
	glViewport(0, 0, gl_width, gl_height);

/*-------------------------------RENDERING LOOP-------------------------------*/
	bool loop = true;
	SDL_Event event;

	// control keys
	bool cam_moved = false;
	Vec3f move (0.0, 0.0, 0.0);
	float cam_yaw = 0.0f; // y-rotation in degrees
	float cam_pitch = 0.0f;
	float cam_roll = 0.0;

	//quaternion
	float q_yaw[4];
	float q_roll[4];
	float q_pitch[4];

	// update timers
	unsigned int previous_time= SDL_GetTicks();;
	unsigned int current_time = previous_time;
	unsigned int elapsed_time = 0;
	double elapsed_seconds;

	while (loop) 
	{
		glClearColor(0.1, 0.1, 0.1, 1.0);
		glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

		// control keys
		cam_moved = false;
		move=Vec3f(0.0, 0.0, 0.0);
		cam_yaw = 0.0f; // y-rotation in degrees
		cam_pitch = 0.0f;
		cam_roll = 0.0;

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
				case SDLK_q:
					move.v[0] -= cam_speed * elapsed_seconds;
					cam_moved = true;
					break;
				case SDLK_d:
					move.v[0] += cam_speed * elapsed_seconds;
					cam_moved = true;
					break;

				case SDLK_a:
					move.v[1] += cam_speed * elapsed_seconds;
					cam_moved = true;
					break;
				case SDLK_e:
					move.v[1] -= cam_speed * elapsed_seconds;
					cam_moved = true;
					break;
				case SDLK_z:
					move.v[2] -= cam_speed * elapsed_seconds;
					cam_moved = true;
					break;
				case SDLK_s:
					move.v[2] += cam_speed * elapsed_seconds;
					cam_moved = true;
					break;

				case SDLK_k:
					move.v[0] -= 100 * cam_speed * elapsed_seconds;
					cam_moved = true;
					break;
				case SDLK_m:
					move.v[0] += 100 * cam_speed * elapsed_seconds;
					cam_moved = true;
					break;

				case SDLK_i:
					move.v[1] += 100*cam_speed * elapsed_seconds;
					cam_moved = true;
					break;
				case SDLK_p:
					move.v[1] -= 100*cam_speed * elapsed_seconds;
					cam_moved = true;
					break;

				case SDLK_o:
					move.v[2] -= 100* cam_speed * elapsed_seconds;
					cam_moved = true;
					break;
				case SDLK_l:
					move.v[2] += 100* cam_speed * elapsed_seconds;
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
					cam_yaw += cam_heading_speed * elapsed_seconds;
					cam_moved = true;
		
					// create a quaternion representing change in heading (the yaw)
					create_versor(q_yaw, cam_yaw, up.v[0], up.v[1], up.v[2]);
					// add yaw rotation to the camera's current orientation
					mult_quat_quat(quaternion, q_yaw, quaternion);
		
					// recalc axes to suit new orientation
					quat_to_mat4(R.r, quaternion);
					fwd = R * Vec4f(0.0, 0.0, -1.0, 0.0);
					rgt = R * Vec4f(1.0, 0.0, 0.0, 0.0);
					up =  R * Vec4f(0.0, 1.0, 0.0, 0.0);
					break;
				case SDLK_RIGHT:
					cam_yaw -= cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					create_versor(q_yaw, cam_yaw, up.v[0], up.v[1], up.v[2]);
					mult_quat_quat(quaternion, q_yaw, quaternion);
					
					// recalc axes to suit new orientation
					quat_to_mat4 (R.r, quaternion);
					fwd = R * Vec4f(0.0, 0.0, -1.0, 0.0);
					rgt = R * Vec4f(1.0, 0.0, 0.0, 0.0);
					up = R * Vec4f(0.0, 1.0, 0.0, 0.0);
					break;

				//pitch
				case SDLK_UP:
					cam_pitch += cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					create_versor(q_pitch, cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]);
					mult_quat_quat(quaternion, q_pitch, quaternion);
		
					// recalc axes to suit new orientation
					quat_to_mat4 (R.r, quaternion);
					fwd = R * Vec4f(0.0, 0.0, -1.0, 0.0);
					rgt = R * Vec4f(1.0, 0.0, 0.0, 0.0);
					up = R * Vec4f(0.0, 1.0, 0.0, 0.0);
					break;


				case SDLK_DOWN:
					cam_pitch -= cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					create_versor(q_pitch, cam_pitch, rgt.v[0], rgt.v[1], rgt.v[2]);
					mult_quat_quat(quaternion, q_pitch, quaternion);
		
					// recalc axes to suit new orientation
					quat_to_mat4 (R.r, quaternion);
					fwd = R * Vec4f(0.0, 0.0, -1.0, 0.0);
					rgt = R * Vec4f(1.0, 0.0, 0.0, 0.0);
					up = R * Vec4f(0.0, 1.0, 0.0, 0.0);
					break;

				// roll
				case SDLK_w:
					cam_roll -= cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					create_versor(q_roll, cam_roll, fwd.v[0], fwd.v[1], fwd.v[2]);
					mult_quat_quat(quaternion, q_roll, quaternion);

					// recalc axes to suit new orientation
					quat_to_mat4 (R.r, quaternion);
					fwd = R * Vec4f(0.0, 0.0, -1.0, 0.0);
					rgt = R * Vec4f(1.0, 0.0, 0.0, 0.0);
					up = R * Vec4f(0.0, 1.0, 0.0, 0.0);
					break;

				case SDLK_c:
					cam_roll += cam_heading_speed * elapsed_seconds;
					cam_moved = true;
					create_versor(q_roll, cam_roll, fwd.v[0], fwd.v[1], fwd.v[2]);
					mult_quat_quat(quaternion, q_roll, quaternion);
		
					// recalc axes to suit new orientation
					quat_to_mat4(R.r, quaternion);
					fwd = R * Vec4f(0.0, 0.0, -1.0, 0.0);
					rgt = R * Vec4f(1.0, 0.0, 0.0, 0.0);
					up = R * Vec4f(0.0, 1.0, 0.0, 0.0);
					break;

				default:
					break;
				}
			}
		}

		// update view matrix
		if (cam_moved) {
			quat_to_mat4(R.r, quaternion);
			// checking for fp errors
			//	printf ("dot fwd . up %f\n", dot (fwd, up));
			//	printf ("dot rgt . up %f\n", dot (rgt, up));
			//	printf ("dot fwd . rgt\n %f", dot (fwd, rgt));

			cam_pos = cam_pos + Vec3f(fwd) * -move.v[2];
			cam_pos = cam_pos + Vec3f(up) * move.v[1];
			cam_pos = cam_pos + Vec3f(rgt) * move.v[0];
			Mat4f T = Mat4f::translation(Vec3f(cam_pos));

			view_mat = R.inverse() * T.inverse();
		}

		//~ model_mat = model_mat * Mat4f::rotation(Vec3f(1.0f,0.0f,0.0f), 0.01f);

		normal_mat = ((view_mat * model_mat).inverse()).transpose();

		shaderOJM->use();
		shaderOJM->setUniform("ModelViewMatrix" , view_mat * model_mat);
		shaderOJM->setUniform("ProjectionMatrix" , proj_mat);
		shaderOJM->setUniform("NormalMatrix" , normal_mat);
		shaderOJM->setUniform("MVP" , proj_mat * view_mat * model_mat);

		shaderOJM->setUniform("Light.Position" , view_mat *  Vec4f(0.0, 0.0, 0.0, 1.0));
		shaderOJM->setUniform("Light.Intensity" , lum*Vec3f(1.0, 1.0, 1.0));

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
