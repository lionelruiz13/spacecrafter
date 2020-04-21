#ifndef APP_HPP
#define APP_HPP

class AppCommandInterface;
class Ui;
class Core;

class App {

public:
	App();
	~App();

    void init();

    void update();

	void draw();

private:

    AppCommandInterface * commander = nullptr;
    Ui * ui = nullptr;
    Core* core = nullptr;

};

#endif