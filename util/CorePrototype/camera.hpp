#ifndef CAMERA_HPP
#define CAMERA_HPP

#include<iostream>
#include<string>

class Camera {

public:
	Camera(int p, int t): pan(p), tilt(t) {}
	~Camera() {}

    void getCamera() {
        std::cout << "pan = " + std::to_string(pan) + " ; tilt = " + std::to_string(tilt) << std::endl;
    }

	void setCamera(int p, int t){
        pan = p;
        tilt = t;
    }

private:
    int pan;
    int tilt;
};

#endif