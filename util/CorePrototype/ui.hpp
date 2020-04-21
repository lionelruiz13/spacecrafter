#ifndef UI_HPP
#define UI_HPP

class App;
class Core;

#include<iostream>

class Ui {

public:
	Ui(Core* _core, App *_app) {
        core = _core;
        app = _app;
    }
	~Ui() {}

    void init() {
        std::cout << "initialisation ui" << std::endl;
    }

    void update() {
        std::cout << "update ui" << std::endl;
    }

	void draw() {
        std::cout << "draw ui" << std::endl;
    }

private:
    
    Core * core;
    App * app;

};

#endif