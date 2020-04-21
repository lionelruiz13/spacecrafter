#ifndef APP_HPP
#define APP_HPP

#include"core.hpp"
#include"AppCommandInterface.hpp"
#include"ui.hpp"

class App {

public:
	App();

	~App();

    void init();

    void update();

    void draw();

    void start_main_loop();

private:

    AppCommandInterface* commander = nullptr;
    Ui* ui = nullptr;
    Core* core = nullptr;
    
};

#endif