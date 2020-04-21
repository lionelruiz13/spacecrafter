#include"app.hpp"

App::App() {
    core = new Core();
    commander = new AppCommandInterface(core, this, ui);
    ui = new Ui(core, this);

    commander->init();

}

App::~App() {
    delete commander;
    delete core;
    delete ui;
}

void App::init() {
    core->init();
    ui->init();
}

void App::update() {
    ui->update();
    core->update();
}

void App::draw() {
    core->draw();
    ui->draw();
}

void App::start_main_loop() {
    flagVisible = true;

    while(flagVisible){


        this->update();
        this->draw();

        
    }
}