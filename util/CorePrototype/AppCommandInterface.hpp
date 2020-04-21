#ifndef APPCOMMANDINTERFACE_HPP
#define APPCOMMANDINTERFACE_HPP

class Core;
class App;
class Ui;

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