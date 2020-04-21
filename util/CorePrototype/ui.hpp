#ifndef UI_HPP
#define UI_HPP

#include"core.hpp"
class App;

#include<iostream>

class Ui {

public:
	Ui(Core* _core, App *_app) ;
	~Ui();

    void init() ;

    void update();

	void draw();

private:
    
    Core * core;
    App * app;

};

#endif