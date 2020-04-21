#ifndef APP_HPP
#define APP_HPP

#include"core.hpp"
#include"AppCommandInterface.hpp"
#include"ui.hpp"

class App {

public:
	App() {
        core = new Core();
        commander = new AppCommandInterface(core, this, ui);
        ui = new Ui(core, this);
    }

	~App() {
        delete commander;
        delete core;
        delete ui;
    }

    void init() {}

    void update() {}

	void draw();{}

private:

    AppCommandInterface * commander = nullptr;
    Ui * ui = nullptr;
    Core* core = nullptr;

};

#endif