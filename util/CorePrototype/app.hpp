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

        commander->init();

    }

	~App() {
        delete commander;
        delete core;
        delete ui;
    }

    void init() {
        core->init();
        ui->init();
    }

    void update() {
        ui->update();
        core->update();
    }

	void draw() {
        core->draw();
        ui->draw();
    }

    void start_main_loop();

private:

    AppCommandInterface* commander = nullptr;
    Ui* ui = nullptr;
    Core* core = nullptr;
    
};

#endif