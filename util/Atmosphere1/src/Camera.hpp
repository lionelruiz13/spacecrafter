#ifndef CAMERA_HPP_INCLUDED
#define CAMERA_HPP_INCLUDED

#include "main.hpp"
#include "vecmath.hpp"

/**
*   @file Camera.hpp Camera.cpp
*   @date 29/12/2015
*   @version 1.0
*   @author Jérôme Lartillot
*
*   @section Requirements Requirements
*
*   -SDL2
*   -vecmath
*   -glu (just if you use the second lookAt function)
*   -sstream
*
*
*   @section Description Description
*
*   This class contains everything to manage your camera.
*   To get started, just use Camera with default parameters,
*   then use his listen and update functions.
*   Have a look at all the parameters you can modify, such as transition
*   behavior, duration, sensibility, etc...
*
*
*   @section Installation Installation and example
*
*   First create a Camera object next to your main loop and initialize it.
*   You may need to create some other objects to do so.
*   Here is an example about how to create a basic one at the origin of your landmark.
*
*    Mat4f campos=Mat4f::identity();
*
*    CameraDefaultListener mdl;
*
*    CameraScale mvs(-1,-20,10,0,Transitionf::defaultZoom());
*
*    Camera Camera(campos,
*                    Transitionf::defaultMovement(),
*                    &mdl,
*                    mvs,
*                    1.0,true,0.08,4500,1024,0.0,0.0
*                    );
*
*   An other example is of course
*
*   Camera Camera;
*
*   or even
*
*   Camera Camera("config.txt");
*
*   Then go to your SDL's event recovery part and add after
*
*   while(SDL_PollEvent(&event))
*    {
*
*   Call the listen function:
*
*       Camera.listen(event);
*
*   After that, go at the end of event recovery, in your update part,
*   and update the camera before any use of it:
*   Camera.update();
*
*   Finally, use it!
*   You have 2 ways to do so:
*   -call the old gluLookAt function
*
*    Camera.lookAt();
*
*   -get the view matrix to feed your shaders:
*
*    view= Camera.getViewMatrix();
*
*   @section Tips Installation tips
*
*   Adjust the sensitivity of the mouse and the speed of the joystick.
*   Test them before.
*   Choose carefully your margin on poles for the joystick.
*   5000 is the minimal value to avoid turning while in a rest state,
*   but if you can have a smaller value it's better, or maybe you want 6000/7000
*   to be safe?
*   If you prefer the joystick, a smaller amount of zoom steps will allow you to press less buttons
*   and use the software easier. So before setting the steps number, check if you need them.
*
*   @section Placement Placing the camera:
*
*   There are 3 placing functions:
*   -void setPos(const Mat4f&m);
*   -void startChasing(const Mat4f&m);
*   -void chase(const Mat4f&m);
*
*   The first one is to move the camera to a static object (using specified transition).
*   Just use it once, update() will do the rest!
*
*   Now if you want the camera to follow an object:
*   First change the target by using startChasing once,
*   then update his position to the camera class using chase.
*   Don't forget to call camera.chase before camera.update, or your camera will be late...
*
*   @section Debugging Debugging
*
*   If you see nothing but your background color (generally black)
*   then check your initialization, you probably forgot a parameter
*   or put a wrong value.
*
*   Mouse or joystick parameters: Camera
*.
*   Wrong inputs/no inputs: listener (CameraDefaultListener or your personalized listener).
*
*   The camera moves too fast/too slowly: check the Camera transition
*
*   It goes too far when i scroll/i have to scroll too much/not enough:
*   check the CameraScale transition.
*
*   @section Transitions Writing your own transition function:
*
*   A transition function float func (float a,float b,float t) have to tell
*   how fast we move from a point to a other through time.
*   It takes value in [0.0;1.0] for the third parameter, where 0.0 is the begin of the transition and
*   1.0 is the end of the transition, and gives value in [0.0;1.0], converted as[a;b] where
*   0.0 is the first point and 1.0 is the second one (the destination).
*   For the output, 0.1 mean i'm closer to the first point,
*   but also a little bit to the second point.
*   When you computed the value (v) in [0.0;1.0],
*   just do a*(1.0-v)+b*v to get the final result.
*   You can do this by calling linear.
*
*/

#define REVERT_TRUE (-1.0)
#define REVERT_FALSE (1.0)

inline float instant            (float a, float b, float t);
inline float linear             (float a, float b, float t);
inline float smooth             (float a, float b, float t);
inline float smoother           (float a, float b, float t);
inline float quickAndSmoother   (float a, float b, float t);
//,
//float   revertMouseX    =REVERT_FALSE,   float revertMouseY    =REVERT_FALSE,
//float   revertJoystickX =REVERT_TRUE,    float revertJoystickY =REVERT_FALSE
typedef Uint Time_; // just for Transitionf lisibility


typedef float (*TransitionFunc)(float,float,float);
/**
*   @struct Transitionf
*   To describe a transition using floating values.
*   For initialization: just set duration and func!
*/
struct Transitionf {
	Time_ duration; // milliseconds
	Time_ begin; // begin of the transition
	float t;
	float(*func)(float,float,float); // function to use (linear, smooth, or smoother)

	static const Transitionf defaultMovement();
	static const Transitionf defaultZoom();
	inline void  update();
	static const TransitionFunc fromString(const std::string&source);
	inline bool isOver()const;
};

/**
*   @struct CameraInput
*   Defines inputs for the Camera class.
*   There are mouse and joystick as field names,
*   But it may be supplied by other input devices or functions.
*   Mouse will turn at PI/mouseSensitivity rad/unit.
*   Joystick will turn at joySpeed rad/sec when fully pushed.
*/
struct CameraInput {
	int relativeMouseX;
	int relativeMouseY;
	int joystickX;
	int joystickY;
	int joystick2X;
	int joystick2Y;
	bool further;
	bool closer;
};

/**
*   @class CameraListener
*    Defines a Camera listener class.
*   Contain a function that supply input from an SDL_Event variable.
*/
class CameraListener {
public:
	CameraInput i;
	virtual inline void prepare();
	virtual inline const CameraInput listen(const SDL_Event&e);
};

/**
*   @class CameraDefaultListener
*   Use mouse, mouse wheel, and xbox controller to feed inputs.
*/
class CameraDefaultListener:public CameraListener {
	SDL_Joystick*joy;


public:
	CameraDefaultListener();
	virtual ~CameraDefaultListener();
	inline void prepare();
	inline const CameraInput listen(const SDL_Event&e);
};

/**
*   @class CameraScale
*   Manage the distance between the target and the camera.
*   It'll be nbSteps steps between begin (nearest) and end (farthest),
*   with step the current step.
*/
class CameraScale {
	friend class Camera;
public:
	CameraScale(float b,float e,Ushort nbs,Ushort s,const Transitionf&t );
	const float getDistance(const Ushort step)const;
	void further();
	void closer();
	void update();
	const float getCrtDist()const;

	static CameraScale getDefault();

private:
	float begin;
	float end;
	Ushort nbSteps;

	float oldDist;
	float crtdist; // Current distance between the target and the camera.
	Ushort step; // Current desired step.
	Transitionf transition;
};


/**
*   @class Camera
*   Class to look around an object.
*/
class Camera {
public:

	Camera(const   Mat4f&position= Mat4f::identity(),
	       const   Transitionf&mt= Transitionf::defaultMovement(),
	       CameraListener*l= new CameraDefaultListener(),
	       const   CameraScale&mvs= CameraScale::getDefault(),
	       float   joystickSpeed= 1.0,
	       bool    enableJoystickPoleLimit=true,
	       float   joystickPoleMargin= 0.08,
	       short   joystickRestMargin= 6000,
	       float   mouseSensitivity = 1024,
	       double  hangle= 0.0,
	       double  vangle= 0.0,
	       float   revertMouseX    =REVERT_FALSE,   float revertMouseY     =REVERT_FALSE,
	       float   revertJoystickX =REVERT_FALSE,   float revertJoystickY  =REVERT_FALSE
	      );
	Camera(const std::string& configFileName, CameraListener*l= new CameraDefaultListener() );
	virtual ~Camera();

	void setPos(const Mat4f&m);
	void startChasing(const Mat4f&m);
	void chase(const Mat4f&m);

	const Mat4f getViewMatrix()const;
	const Mat4f getViewBeforeLookAtMatrix()const;
	void lookAt()const;

	void listen(const SDL_Event&e);
	void update();


protected:
	// Angles

	double moveHorAngle; // horizontal angle (rad) (moving  camera)
	double moveVerAngle; // vertical   angle (rad) (moving  camera)
	double tmhax; // temporary move horizontal angle on x.
	double tmhay; // temporary move horizontal angle on y.
	double turnHorAngle; // horizontal angle (rad) (turning camera)
	double turnVerAngle; // vertical   angle (rad) (turning camera)
	double hcos,hsin,vcos,vsin;

	// camera vectors
	Vec4f camPosVec; // camera position vector
	Vec4f tgt; // camera target
	Vec4f up; // up vector

	Mat4f baseMatrix;
	Mat4f moveMatrix; // move around
	Mat4f turnMatrix; // look around
	Mat4f horizontalOrientation;
	Mat4f viewBeforeLookAt;
	Mat4f viewMatrix;

	// datas
	CameraListener*listener;
	CameraScale scale;

	Mat4f oldpos; // initial position
	Mat4f crtpos; // current position
	Mat4f pos;    // asked position

	Transitionf transition; // movement transition

	float joySpeed;
	bool enableJPM; // enable Joystick Pole Margin
	float joyPoleMargin;
	short restMargin; // rest margin for the joystick position.
	Time_ lastFrame;
	float mouseSensitivity;

	float rjx,rjy,rj2x,rj2y,rmx,rmy; // Revert Joystick(left 1/ right 2)/Mouse X/Y
};


#define CAMERAMODE_LOOKAROUND      0
#define CAMERAMODE_RELATIVEFREEFLY 1
#define CAMERAMODE_FREEFLY         2

/**
*   @class SiriusCamera
*   Class to move the camera like we would like to do it on SpaceCrafter.
*/
class SiriusCamera:public Camera {
private:

	// pos = position of the chased object
	// view Matrix = camera position

	std::vector<std::vector<Mat4f> >* matrixArray;
	int planetCursor;
	int  moonCursor;
	int oldPlanetCursor;
	int oldMoonCursor;

	int cameraMode;
	Mat4f relativePosition;
	Mat4f absolutePosition;
	bool straighten;
	float freeflySpeed;

public:
	SiriusCamera(const   Mat4f&position= Mat4f::identity(),
	             const   Transitionf&mt= Transitionf::defaultMovement(),
	             CameraListener*l= new CameraDefaultListener(),
	             const   CameraScale&mvs= CameraScale::getDefault(),
	             float   joystickSpeed= 1.0,
	             bool    enableJoystickPoleLimit=true,
	             float   joystickPoleMargin= 0.08,
	             short   joystickRestMargin= 6000,
	             float   mouseSensitivity = 1024,
	             double  hangle= 0.0,
	             double  vangle= 0.0,
	             float   revertMouseX    =REVERT_FALSE,   float revertMouseY     =REVERT_FALSE,
	             float   revertJoystickX =REVERT_FALSE,   float revertJoystickY  =REVERT_FALSE
	            );
	SiriusCamera(const std::string& configFileName, CameraListener*l= new CameraDefaultListener() );

	void init(std::vector<std::vector<Mat4f> >* matArray,int startCursor);
	void listen(const SDL_Event&e);
	void update(); // call the movement function + the update function.

	Mat4f getViewMatrix()const;
};

#endif // Camera_HPP_INCLUDED






























