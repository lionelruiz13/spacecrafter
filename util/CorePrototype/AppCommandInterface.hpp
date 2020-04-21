#ifndef APPCOMMANDINTERFACE_HPP
#define APPCOMMANDINTERFACE_HPP

#include"core.hpp"
#include"app.hpp"
#include"ui.hpp"

class AppCommandInterface {

public:
	AppCommandInterface();
	~AppCommandInterface();

    void init();

    void update();

	void draw();

private:

	Core * stcore = nullptr;
	App * stapp = nullptr;
	Ui* ui = nullptr;
};

#endif