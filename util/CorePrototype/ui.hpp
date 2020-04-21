#ifndef UI_HPP
#define UI_HPP

#include"core.hpp"
#include"AppCommandInterface.hpp"
#include"app.hpp"


class Ui {

public:
	Ui(Core * _core, App * _app) {
        core = _core;
        app = _app;
    }
	~Ui() {}

    void init() {}

    void update() {}

	void draw() {}

private:
    
    Core * core;
    App * app;

};

#endif