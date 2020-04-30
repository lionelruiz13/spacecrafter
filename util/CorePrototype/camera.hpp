#ifndef CAMERA_HPP
#define CAMERA_HPP

#include<iostream>
#include<string>

class Camera {

public:
	Camera(std::string n): name(n) {}
	~Camera() {}

    void getCamera() {
        std::cout << "Camera " + name << std::endl;
    }

	void setCamera(std::string n){
        name = n;
    }

private:
    std::string name;
};

#endif