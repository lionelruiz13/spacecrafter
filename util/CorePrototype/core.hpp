#ifndef CORE_HPP
#define CORE_HPP

#include<iostream>

class Core {

public:
	Core() {}
	~Core() {}

    void init() {
        std::cout << "initialisation core" << std::endl;
    }

    void update() {
        std::cout << "update core" << std::endl;
    }

	void draw() {
        std::cout << "draw core" << std::endl;
    }

private:

};

#endif