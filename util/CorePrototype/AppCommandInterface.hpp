#ifndef APPCOMMANDINTERFACE_HPP
#define APPCOMMANDINTERFACE_HPP

#include"core.hpp"
class App;
class Ui;

#include<iostream>

class AppCommandInterface {

public:
	AppCommandInterface(Core * _core,  App * _app, Ui* _ui );
	~AppCommandInterface();

    void init();

    void update();

	void draw();

private:

	Core * core = nullptr;
	App * app = nullptr;
	Ui* ui = nullptr;
};

#endif