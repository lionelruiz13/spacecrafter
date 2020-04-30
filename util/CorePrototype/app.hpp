#ifndef APP_HPP
#define APP_HPP

#include"CoreMode.hpp"
#include"CoreMode1.hpp"
#include"CoreMode2.hpp"
#include"CoreMode3.hpp"
#include"core.hpp"
#include"coreLink.hpp"
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
    CoreLink* coreLink = nullptr;
    
    CoreMode1 * pCore1=nullptr;
    CoreMode2 * pCore2=nullptr;
    CoreMode3 * pCore3=nullptr;
    CoreMode* pCore=nullptr;
};

#endif