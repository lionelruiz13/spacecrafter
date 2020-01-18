#include "Camera.hpp"

/**
*   Instant jump to b, no interpolation.
*   @param a first value
*   @param b second value
*   @param t advancement
*   @return b.
*/
inline float instant(float a,float b, float t)
{
	return b;
}

/**
*   Linear interpolation between a and b, coef t
*   @param a first value
*   @param b second value
*   @param t advancement
*   @return The result linearly.
*/
inline float linear(float a,float b, float t)
{
	return a*(1.0-t)+b*t;
}
/**
*   Smooth interpolation between a and b, coef t
*   @param a first value
*   @param b second value
*   @param t advancement
*   @return The result smoothly.
*/
inline float smooth(float a, float b, float t)
{
	return linear(a,b,t*t*(3.0-2.0*t));
}
/**
*   Smoother interpolation between a and b, coef t
*   @param a first value
*   @param b second value
*   @param t advancement
*   @return The result, but even more smoothly.
*/
inline float smoother(float a,float b,float t)
{
	return linear(a, b, t*t*t*(10+t*(t*6-15)));
}

/**
*   Quick and smoother interpolation between a and b, coef t
*   Start fast, end smoothly.
*   @param a first value
*   @param b second value
*   @param t advancement
*   @return The result, but quick and even more smoothly.
*/
inline float quickAndSmoother(float a,float b,float t)
{
	return smoother(a,b,t*0.5+0.5);
}


/**
*   Gives a default movement transition.
*   Default values: a 3 seconds nicely smooth transition.
*   @return a default Transitionf.
*/
const Transitionf Transitionf::defaultMovement()
{
	Transitionf tf;
	tf.duration=5000;
	tf.func=&smoother;
	//tf.func=&smooth;
	return tf;
}


/**
*   Gives a default zoom transition.
*   Default values: a 0.25 seconds transition which starts fast but end smoothly.
*   @return a default Transitionf.
*/
const Transitionf Transitionf::defaultZoom()
{
	Transitionf tf;
	tf.duration=250;
	tf.func=&quickAndSmoother;
	return tf;
}



/**
*   Compute the new value of the transition's advancement.
*   Store this value in t.
*   t belong to [0,1].
*/
void Transitionf::update()
{
	if(!isOver())
		t= ((float)SDL_GetTicks()-(float)begin)/duration;
	else t= 1.0;
}

/**
*   Convert a string into a transition advancement function.
*   @param source the string to convert.
*   Are accepted: INSTANT LINEAR SMOOTH SMOOTHER QUICKANDSMOOTHER
*   @return The corresponding function pointer.
*/
const TransitionFunc Transitionf::fromString(const std::string&source)
{
	if(source.find("INSTANT")!=std::string::npos) return &instant;
	if(source.find("LINEAR")!=std::string::npos) return &linear;
	if(source.find("SMOOTH")!=std::string::npos) return &smooth;
	if(source.find("SMOOTHER")!=std::string::npos) return &smoother;
	if(source.find("QUICKANDSMOOTHER")!=std::string::npos) return &quickAndSmoother;
}

inline bool Transitionf::isOver()const
{
	return (SDL_GetTicks()-begin>duration);
}

/**
*   This method intend to be herited.
*   It'll be called after update (before event loop)
*   It may be useless but because i need it for default listener it exist.
*   It's intended to reset variables before the next loop.
*/
inline void CameraListener::prepare()
{
	exit(EXIT_FAILURE);
}

/**
*   This method intend to be herited.
*/
const CameraInput CameraListener::listen(const SDL_Event&e)
{
	exit(EXIT_FAILURE);
}

/**
*   CameraDefaultListener default constructor.
*   Set default values.
*   Set restMargin to 6000, a comfortable value.
*/
CameraDefaultListener::CameraDefaultListener()
{
	if(SDL_NumJoysticks()>0) {
		joy=SDL_JoystickOpen(0);
		//cout<<"joystick opened!"<<endl;
		if(joy==NULL)
			std::cout<<"joy opening failed"<<std::endl;
	} else
		joy=NULL;


	prepare();
	i.joystickX=0;
	i.joystickY=0;
	i.joystick2X=0;
	i.joystick2Y=0;
}

/**
*   Prepare the listener for the event loop.
*   This function is just to reset values before the event loop.
*/
void CameraDefaultListener::prepare()
{
	i.relativeMouseX=0;
	i.relativeMouseY=0;
	i.closer=false;
	i.further=false;
}

/**
*   Destroy the listener (just close the joystick)
*/
CameraDefaultListener::~CameraDefaultListener()
{
	if (joy!=NULL)
		if (SDL_JoystickGetAttached(joy))
			SDL_JoystickClose(joy);
	delete joy;
}

/**
*   Listen events to get a CameraInput for the Camera class.
*   @return input values.
*/

static int var=0;
const CameraInput CameraDefaultListener::listen(const SDL_Event&e)
{
	switch(e.type) {
		// Mouse
		case SDL_MOUSEMOTION:
			i.relativeMouseX=e.motion.xrel;
			i.relativeMouseY=-e.motion.yrel;
			break;
		case SDL_MOUSEWHEEL:
			if(e.wheel.y>0)
				i.closer=true;
			if(e.wheel.y<0)
				i.further=true;
			break;
		// Joystick
		case SDL_JOYDEVICEADDED:
			if(SDL_NumJoysticks()>0) {
				joy=SDL_JoystickOpen(0);
				//cout<<"joystick opened!"<<endl;
				if(joy==NULL)
					std::cout<<"joy opening failed"<<std::endl;
			} else
				joy=NULL;
			break;
		case SDL_JOYDEVICEREMOVED:
			if (joy!=NULL)
				if (SDL_JoystickGetAttached(joy))
					SDL_JoystickClose(joy);
			break;
		case SDL_JOYAXISMOTION:
			switch(e.jaxis.axis) {
				case 0:
					i.joystickX = e.jaxis.value;
					break; // horizontal axis left  joystick (xbox 360 controller)
				case 1:
					i.joystickY = -e.jaxis.value;
					break; // vertical   axis left  joystick (xbox 360 controller) (- because CCW)
				case 2:
					i.joystick2X= e.jaxis.value;
					break; // horizontal axis right joystick (xbox 360 controller)
				case 3:
					i.joystick2Y= -e.jaxis.value;
					break; // vertical   axis right joystick (xbox 360 controller) (- because CCW)
			}

			break;
		case SDL_JOYBUTTONDOWN:
			switch(e.jbutton.button) {
				case 10:
					i.closer =true;
					break; // a button (xbox 360 controller)
				case 11:
					i.further=true;
					break; // b button (xbox 360 controller)
			}
			break;
	}
	return i;
}

/**
*   CameraScale constructor.
*   @param begin the nearest distance between eye and object.
*   @param end the farthest distance between eye and object.
*   @param nbsteps the step number between begin and end.
*   @param step the current step (belong to [0,step+1]).
*   @param transition the transition behavior. instant, linear and quickAndSmoother are good.
*/
CameraScale::CameraScale(float b,float e,Ushort nbs,Ushort s,const Transitionf&t):
	begin(b),
	end(e),
	nbSteps(nbs),
	oldDist(getDistance(s)),
	crtdist(getDistance(s)),
	step(s),
	transition(t)
{}

/**
*   Return the distance between the model and the specified step step.
*   @param step the step to get the distance from.
*   @return The distance.
*/
const float CameraScale::getDistance(const Ushort step)const
{
	//return linear(begin,end,(float)step/(nbSteps+1));
	return begin+(end-begin)*(float)step/(nbSteps+1);
}

/**
*   Move further, one step.
*/
void CameraScale::further()
{
	if(step<nbSteps+1) {
		oldDist=crtdist;
		step++;
		transition.begin=SDL_GetTicks();
	}
}

/**
*   Move closer, one step.
*/
void CameraScale::closer()
{
	if(step>0) {
		oldDist=crtdist;
		step--;
		transition.begin=SDL_GetTicks();
	}
}

/**
*   Update crtdist according to transition parameters.
*/
void CameraScale::update()
{
	transition.update();
	crtdist=transition.func(oldDist,getDistance(step),transition.t);
}

/**
*   Gives the current distance between the object and the camera.
*   @return The distance.
*/
const float CameraScale::getCrtDist()const
{
	return crtdist;
}

/**
*   Return a default CameraScale.
*   @return cf description.
*/
CameraScale CameraScale::getDefault()
{
	CameraScale mvs(1,50,32,0,Transitionf::defaultZoom());
	return mvs;
}

/**
*   Camera constructor
*   @param position the initial matrix of the target to see (model matrix without scale).
*   or consider model matrix scale while setting scale values.
*   @param transition the transition configuration for new positions.
*   @param listener the input class (and also function) to use to get inputs.
*   @param CameraScale a class with all scale parameters to move closer or further.
*   @param joystickSpeed the joystick speed, in rad/sec.
*   @param enableJoystickPoleLimit to know whether we should stop the camera or let you reverse it on poles.
*   @param joystickPoleMargin the joystick pole margin, usually in [0.02,0.2],
*   determines how close to the poles the camera may go.
*   @param joystickRestMargin determines the range where the joystick won't affect the camera.
*   7000-6000-5000-4500 are good values but you can adjust it to your controller.
*   @param mouseSensitivity the mouse sensitivity, the step number in 1 rad.
*   @param hAngle the initial value of the horizontal angle.
*   @param vAngle the initial value of the vertical angle.
*   @param revertMouseX REVERT_TRUE or REVERT_FALSE
*   @param revertMouseY REVERT_TRUE or REVERT_FALSE
*   @param revertJoystickX REVERT_TRUE or REVERT_FALSE
*   @param revertJoystickY REVERT_TRUE or REVERT_FALSE
*/
Camera::Camera(   const Mat4f&position,const Transitionf&t,
                  CameraListener*l,const CameraScale&mvs,
                  float   joystickSpeed,
                  bool    enableJoystickPoleLimit,
                  float   joystickPoleMargin,
                  short   joystickRestMargin,//!
                  float   mouseSensitivity,
                  double  hangle,
                  double  vangle,
                  float   revertMouseX   ,   float revertMouseY,
                  float   revertJoystickX,   float revertJoystickY
              ):
	listener(l),
	transition(t),
	scale(mvs),
	oldpos(position),
	crtpos(position),
	pos(position),
	moveHorAngle(hangle),
	moveVerAngle(vangle),
	joySpeed(joystickSpeed),
	enableJPM(enableJoystickPoleLimit),
	joyPoleMargin(joystickPoleMargin),
	restMargin(joystickRestMargin),
	mouseSensitivity(mouseSensitivity),
	lastFrame(0),
	camPosVec(position.getVector(3)),
	tgt(cos(moveHorAngle)*cos(moveVerAngle),sin(moveHorAngle)*cos(moveVerAngle),sin(moveVerAngle)),
	rmx(revertMouseX),    rmy(revertMouseY),
	rjx(revertJoystickX), rjy(revertJoystickY)
{
	baseMatrix=Mat4f::identity();
	baseMatrix.set(0,0,1,0,   0,-1,0,0,  1,0,0,0,  0,0,0,1);
}

bool StringToBool(const std::string&s)
{
	if(s.find("OUI")!=std::string::npos) return true;
	if(s.find("JA")!=std::string::npos) return true;
	if(s.find("YES")!=std::string::npos) return true;
	if(s.find("SI")!=std::string::npos) return true;     // italian/spanish
	if(s.find("HAI")!=std::string::npos) return true;
	if(s.find("DA")!=std::string::npos) return true;
	if(s.find("YE")!=std::string::npos) return true;     // korean
	if(s.find("YO")!=std::string::npos) return true;     // lux
	if(s.find("SHI DE")!=std::string::npos) return true; // chinese

	if(s.find("NON")!=std::string::npos) return false;
	if(s.find("NEIN")!=std::string::npos) return false;
	if(s.find("NO")!=std::string::npos) return false;
	if(s.find("NO")!=std::string::npos) return false;
	if(s.find("IIE")!=std::string::npos) return false;
	if(s.find("HE")!=std::string::npos) return false;
	if(s.find("ANI")!=std::string::npos) return false;
	if(s.find("NET")!=std::string::npos) return false;
	if(s.find("BU")!=std::string::npos) return false;

	std::cerr<<"Camera configuration file: couldn't recognize "<<s<<std::endl;
	exit(EXIT_FAILURE);
}

Camera::Camera(const std::string& configFileName , CameraListener*l ):
	listener(l),
	scale(CameraScale::getDefault())
{


	std::ifstream config( configFileName.c_str() );
	size_t equals=-1;
	std::string errors;

	Mat4f position=Mat4f::identity();


	if ( !config.is_open() ) {
		std::cerr<<" Camera constructor from file: unable to open "<<configFileName<<std::endl;
	} else {
		int i=0;
		std::string line;
		//stringstream ss;

		while ( getline( config, line ) ) {
			if(line[0]!='#') {

				equals=line.find('=');
				if(equals!=std::string::npos) {
					//cout<<line<<endl;
					std::stringstream ss(line.substr(equals+1));
					i++;

					switch(i) {
						case 1:
							ss>>position.r[12];
							break;
						case 2:
							ss>>position.r[13];
							break;
						case 3:
							ss>>position.r[14];
							break;

						case 4:
							ss>>transition.duration;
							break;
						case 5:
							transition.func = Transitionf::fromString(line.substr(equals+1));
							break;

						case 6:
							ss>>scale.begin;
							break;
						case 7:
							ss>>scale.end;
							break;
						case 8:
							ss>>scale.nbSteps;
							break;
						case 9:
							ss>>scale.step;
							break;

						case 10:
							ss>>scale.transition.duration;
							break;
						case 11:
							scale.transition.func = Transitionf::fromString(line.substr(equals+1));
							break;

						case 12:
							ss>>joySpeed;
							break;
						case 13:
							enableJPM= StringToBool(line.substr(equals+1));
							break;
						case 14:
							ss>>joyPoleMargin;
							break;
						case 15:
							ss>>restMargin;
							break;

						case 16:
							ss>>mouseSensitivity;
							break;
						case 17:
							ss>>moveHorAngle;
							break;
						case 18:
							ss>>moveVerAngle;
							break;
						case 19:
							ss>>turnVerAngle;
							break;
						case 20:
							ss>>turnHorAngle;
							break;
						case 21:
							rmx=  StringToBool(line.substr(equals+1))?REVERT_TRUE:REVERT_FALSE;
							break;
						case 22:
							rmy=  StringToBool(line.substr(equals+1))?REVERT_TRUE:REVERT_FALSE;
							break;
						case 23:
							rjx=  StringToBool(line.substr(equals+1))?REVERT_TRUE:REVERT_FALSE;
							break; // left joystick, moving
						case 24:
							rjy=  StringToBool(line.substr(equals+1))?REVERT_TRUE:REVERT_FALSE;
							break;
						case 25:
							rj2x= StringToBool(line.substr(equals+1))?REVERT_TRUE:REVERT_FALSE;
							break; // right joystick, turning
						case 26:
							rj2y= StringToBool(line.substr(equals+1))?REVERT_TRUE:REVERT_FALSE;
							break;
					} // switch
				} // if(equals!=string::npos)
			} //if(line[0]!='#')

		} //while ( getline( config, line ) )


		if (i!=26) {
			std::cerr<<"Camera: input configuration file has "<<i<<" items instead of 24!"<<std::endl;
			exit(EXIT_FAILURE);
		}
		oldpos=position;
		crtpos=position;
		pos=position;
		lastFrame=0;
		camPosVec=position.getVector(3);
		tgt.set(cos(moveHorAngle)*cos(moveVerAngle),sin(moveHorAngle)*cos(moveVerAngle),sin(moveVerAngle),1.0);
		scale.oldDist=scale.getDistance(scale.step);
		scale.crtdist=scale.getDistance(scale.step);

	} //if ( !config.is_open() ) else


	baseMatrix=Mat4f::identity();
	baseMatrix.set(0,1,0,0,   0,0,1,0,  -1,0,0,0,  0,0,0,1);
	horizontalOrientation=Mat4f::identity();
}

/**
*   Camera destructor.
*/
Camera::~Camera()
{
	delete listener;
}

/**
*   Set the new position of the object to watch.
*   The camera will go progressively to the point using
*   the transition parameters.
*   @param matrix the new position.
*/
void Camera::setPos(const Mat4f&m)
{
	transition.begin=SDL_GetTicks();
	oldpos=crtpos;
	pos=m;
}

/**
*   Start a chase.
*   If you want to look around a moving object,
*   start by calling this function.
*   The camera will begin to go to the object,
*   but you'll have to indicate the new position of the object
*   at each frame.
*   To do this, you'll have to use Camera::chase.
*   @param matrix the position of the object to chase and look around.
*/
void Camera::startChasing(const Mat4f&m)
{
	transition.begin=SDL_GetTicks();
	oldpos=crtpos;
	pos=m;
}

/**
*   Update the position of the object you must be chasing.
*   This function must be called on update.
*   You can call it every time, even looking at a static object,
*   But understand that it's a teleportation, so the new position
*   must be near the old position.
*   If you want to teleport, set transition to instant instead.
*   @param matrix the new position of the object you want to chase.
*/
void Camera::chase(const Mat4f&m)
{
	pos=m;
}

/**
*   Gives the current view matrix.
*   Requires previous call of the Camera::update function.
*   @return The view matrix.
*/
const Mat4f Camera::getViewMatrix()const
{
	/*return Mat4f::lookAt(camPosVec.v[0],camPosVec.v[1],camPosVec.v[2],
	                     tgt.v[0],tgt.v[1],tgt.v[2],
	                     up[0],up[1],up[2]);*/
	return viewMatrix;
}

const Mat4f Camera::getViewBeforeLookAtMatrix()const
{
	return viewBeforeLookAt;
}

/**
*   Use gluLookAt to place the camera at the current position (deprecated).
*/
void Camera::lookAt()const
{
	gluLookAt(  camPosVec.v[0],camPosVec.v[1],camPosVec.v[2],
	            tgt.v[0],tgt.v[1],tgt.v[2],
	            up[0],up[1],up[2]);
	gluLookAt(viewMatrix[12],viewMatrix[13],viewMatrix[14],
	          viewMatrix[0],viewMatrix[1],viewMatrix[2],
	          viewMatrix[8],viewMatrix[9],viewMatrix[10]);
}

/**
*   Listen events according to the given CameraListener.
*   @param event the SDL_Event variable to retrieve input from.
*/
void Camera::listen(const SDL_Event&e)
{
	listener->listen(e);
}

/**
*   Apply input modifications, and move camera.
*/
void Camera::update()
{


	if(listener->i.closer)
		scale.closer();
	if(listener->i.further)
		scale.further();

	scale.update();
	transition.update();

	crtpos.r[0] = transition.func(oldpos.r[0], pos.r[0], transition.t);
	crtpos.r[1] = transition.func(oldpos.r[1], pos.r[1], transition.t);
	crtpos.r[2] = transition.func(oldpos.r[2], pos.r[2], transition.t);
	crtpos.r[3] = transition.func(oldpos.r[3], pos.r[3], transition.t);
	crtpos.r[4] = transition.func(oldpos.r[4], pos.r[4], transition.t);
	crtpos.r[5] = transition.func(oldpos.r[5], pos.r[5], transition.t);
	crtpos.r[6] = transition.func(oldpos.r[6], pos.r[6], transition.t);
	crtpos.r[7] = transition.func(oldpos.r[7], pos.r[7], transition.t);
	crtpos.r[8] = transition.func(oldpos.r[8], pos.r[8], transition.t);
	crtpos.r[9] = transition.func(oldpos.r[9], pos.r[9], transition.t);
	crtpos.r[10]= transition.func(oldpos.r[10],pos.r[10],transition.t);
	crtpos.r[11]= transition.func(oldpos.r[11],pos.r[11],transition.t);
	crtpos.r[12]= transition.func(oldpos.r[12],pos.r[12],transition.t);
	crtpos.r[13]= transition.func(oldpos.r[13],pos.r[13],transition.t);
	crtpos.r[14]= transition.func(oldpos.r[14],pos.r[14],transition.t);
	crtpos.r[15]= transition.func(oldpos.r[15],pos.r[15],transition.t);

	crtpos.setAsOrthonormalFromZ();




	// For joysticks:
	//32768 = 2^15 = 0x800
	//1/32768 = 0x0002 ?? (looks like it is 0x0001p1)

	tmhax-=    (listener->i.relativeMouseX*M_PI/mouseSensitivity)*rmx;
	tmhay-=    (listener->i.relativeMouseY*M_PI/mouseSensitivity)*rmy;

	if(listener->i.joystickX > restMargin || listener->i.joystickX < -restMargin||
	        listener->i.joystickY > restMargin || listener->i.joystickY < -restMargin) {
		// we need to reverse x axis to get it unreversed
		//moveHorAngle-= -((float)listener->i.joystickX*0x0.0001p1* joySpeed*M_PI*   (float)((int)SDL_GetTicks()-(int)lastFrame)*0.001)*rjx;
		//moveVerAngle-=  ((float)listener->i.joystickY*0x0.0001p1* joySpeed*M_PI*   (float)((int)SDL_GetTicks()-(int)lastFrame)*0.001)*rjy;

		tmhax= ((float)listener->i.joystickX*0x0.0001p1* joySpeed*M_PI*   (float)((int)SDL_GetTicks()-(int)lastFrame)*0.001)*rjx;
		tmhay= ((float)listener->i.joystickY*0x0.0001p1* joySpeed*M_PI*   (float)((int)SDL_GetTicks()-(int)lastFrame)*0.001)*rjy;
		// horizontal orientation is a rotation on x axis.


	}


	moveHorAngle+= horizontalOrientation.r[5]*tmhax + horizontalOrientation.r[9 ]*tmhay;
	moveVerAngle+= horizontalOrientation.r[6]*tmhax + horizontalOrientation.r[10]*tmhay;
	tmhax=0;
	tmhay=0;


	if(listener->i.joystick2X > restMargin || listener->i.joystick2X < -restMargin||
	        listener->i.joystick2Y > restMargin || listener->i.joystick2Y < -restMargin) {
		// we need to reverse x axis to get it unreversed
		turnHorAngle-= -((float)listener->i.joystick2X*0x0.0001p1* joySpeed*M_PI*   (float)((int)SDL_GetTicks()-(int)lastFrame)*0.001)*rj2x;
		turnVerAngle-=  ((float)listener->i.joystick2Y*0x0.0001p1* joySpeed*M_PI*   (float)((int)SDL_GetTicks()-(int)lastFrame)*0.001)*rj2y;
	}

	lastFrame=SDL_GetTicks();

	if(moveHorAngle> M_PI*2.0) moveHorAngle-=M_PI*2.0;
	if(moveHorAngle< 0)        moveHorAngle+=M_PI*2.0;

	/*if(enableJPM)
	{
	    if(moveVerAngle>  M_PI*0.5-joyPoleMargin) moveVerAngle= M_PI*0.5-joyPoleMargin;
	    if(moveVerAngle< -M_PI*0.5+joyPoleMargin) moveVerAngle=-M_PI*0.5+joyPoleMargin;
	}else{
	    if(moveVerAngle> M_PI*2.0) moveVerAngle-=M_PI*2.0;
	    if(moveVerAngle< 0)        moveVerAngle+=M_PI*2.0;
	}*/


	// matrix calculation:


	horizontalOrientation = Mat4f::rotationX(turnHorAngle);
	turnMatrix= horizontalOrientation * Mat4f::rotationY(-turnVerAngle);
	//turnMatrix= Mat4f::identity();

	moveMatrix= Mat4f::rotationZ(moveHorAngle) * Mat4f::rotationY(-moveVerAngle) ;

	viewMatrix= moveMatrix  * turnMatrix;


	viewMatrix.setVector(-moveMatrix.getVector(0)*scale.getCrtDist(),3);
	viewBeforeLookAt= crtpos  * viewMatrix;
	viewMatrix= Mat4f::lookAtFromMatrix(viewBeforeLookAt);
	/*viewMatrix= Mat4f::lookAt(viewBeforeLookAt.r[12],viewBeforeLookAt.r[13],viewBeforeLookAt.r[14],
	                          viewBeforeLookAt.r[0]+viewBeforeLookAt.r[12],viewBeforeLookAt.r[1]+viewBeforeLookAt.r[13],viewBeforeLookAt.r[2]+viewBeforeLookAt.r[14],
	                          viewBeforeLookAt.r[8],viewBeforeLookAt.r[9],viewBeforeLookAt.r[10]);*/




	listener->prepare();
}



SiriusCamera::SiriusCamera(   const Mat4f&position,const Transitionf&t,
                              CameraListener*l,const CameraScale&mvs,
                              float   joystickSpeed,
                              bool    enableJoystickPoleLimit,
                              float   joystickPoleMargin,
                              short   joystickRestMargin,
                              float   mouseSensitivity,
                              double  hangle,
                              double  vangle,
                              float   revertMouseX   ,   float revertMouseY,
                              float   revertJoystickX,   float revertJoystickY
                          ):
	Camera(position,t ,l,mvs ,joystickSpeed, enableJoystickPoleLimit, joystickPoleMargin,
	       joystickRestMargin, mouseSensitivity,hangle,vangle,revertMouseX,revertMouseY,revertJoystickX,revertJoystickY )
{
	cameraMode= CAMERAMODE_LOOKAROUND;
}


SiriusCamera::SiriusCamera(const std::string& configFileName , CameraListener*l ):
	Camera(configFileName, l)
{
	cameraMode= CAMERAMODE_LOOKAROUND;
}


void SiriusCamera::init(std::vector<std::vector<Mat4f> >* matArray,int startCursor)
{
	planetCursor = startCursor;
	moonCursor=0;
	oldPlanetCursor = planetCursor;
	matrixArray = matArray;
	straighten=false;
	freeflySpeed=8.0;
}
void SiriusCamera::listen(const SDL_Event&e)
{
	Camera::listen(e);
	switch(e.type) {
		case SDL_JOYBUTTONDOWN:
			switch(e.jbutton.button) {
				case 0: // up button (xbox 360 controller)
					if(moonCursor<(*matrixArray)[planetCursor].size()-1) {
						if(transition.isOver()) {
							oldPlanetCursor=planetCursor;
							oldMoonCursor = moonCursor;
						}
						moonCursor++;
						if(transition.isOver())
							startChasing((*matrixArray)[planetCursor][moonCursor]);

					}
					break;
				case 1: // down button (xbox 360 controller)
					if(moonCursor>0) {
						if(transition.isOver()) {
							oldPlanetCursor=planetCursor;
							oldMoonCursor = moonCursor;
						}
						moonCursor--;
						if(transition.isOver())
							startChasing((*matrixArray)[planetCursor][moonCursor]);
					}
					break;
				case 2: // left button, precedent
					if(planetCursor>0) {
						if(transition.isOver()) {
							oldPlanetCursor=planetCursor;
							oldMoonCursor = moonCursor;
						}
						planetCursor--;
						moonCursor=0;
						if(transition.isOver())
							startChasing((*matrixArray)[planetCursor][moonCursor]);
					}

					break;
				case 3: // right button, next
					if(planetCursor<matrixArray->size()-1) {
						if(transition.isOver()) {
							oldPlanetCursor=planetCursor;
							oldMoonCursor = moonCursor;
						}
						planetCursor++;
						moonCursor=0;
						if(transition.isOver())
							startChasing((*matrixArray)[planetCursor][moonCursor]);
					}

					break;

				case 4: // start
					switch(cameraMode) {
						case CAMERAMODE_LOOKAROUND:
							cameraMode= CAMERAMODE_RELATIVEFREEFLY;
							relativePosition=Mat4f::identity();
							if(straighten)
								relativePosition= Mat4f::rotation(relativePosition.getVector(2),Vec3f(0,0,1));
							break;
						case CAMERAMODE_RELATIVEFREEFLY:
							cameraMode= CAMERAMODE_FREEFLY;
							absolutePosition= viewBeforeLookAt * relativePosition;
							if(straighten)
								absolutePosition= Mat4f::rotation(absolutePosition.getVector(2),Vec3f(0,0,1));
							break;
					}
					break;

				case 5: // back
					switch(cameraMode) {
						case CAMERAMODE_RELATIVEFREEFLY:
							cameraMode = CAMERAMODE_LOOKAROUND;
							break;
						case CAMERAMODE_FREEFLY:
							cameraMode = CAMERAMODE_RELATIVEFREEFLY;
							relativePosition=Mat4f::identity();
							if(straighten)
								relativePosition= Mat4f::rotation(relativePosition.getVector(2),Vec3f(0,0,1));
							break;
					}
					break;

			}
			break;
	}
}
void SiriusCamera::update() // call the movement function + the update function.
{
	if(cameraMode==CAMERAMODE_LOOKAROUND) {
		chase( (*matrixArray)[planetCursor][moonCursor]);
		oldpos=(*matrixArray)[oldPlanetCursor][oldMoonCursor];
		Camera::update();
	}
	if(cameraMode==CAMERAMODE_RELATIVEFREEFLY) {
		chase( (*matrixArray)[planetCursor][moonCursor]);
		oldpos=(*matrixArray)[oldPlanetCursor][oldMoonCursor];

	}
	if(cameraMode==CAMERAMODE_FREEFLY) {
		chase( (*matrixArray)[planetCursor][moonCursor]);
		oldpos=(*matrixArray)[oldPlanetCursor][oldMoonCursor];

	}

}

Mat4f SiriusCamera::getViewMatrix()const
{
	switch(cameraMode) {
		case CAMERAMODE_LOOKAROUND:
			return viewMatrix;
			break;
		case CAMERAMODE_RELATIVEFREEFLY:
			return Mat4f::lookAtFromMatrix(viewBeforeLookAt*relativePosition);
			break;
		case CAMERAMODE_FREEFLY:
			return Mat4f::lookAtFromMatrix(absolutePosition);
			break;
	}

}


































