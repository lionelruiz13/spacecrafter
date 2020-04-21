#ifndef UI_HPP
#define UI_HPP

class App;
class Core;
class AppCommandInterface;

class Ui {

public:
	Ui();
	~Ui();

    void init();

    void update();

	void draw();

private:
    
    Core * core;
    App * app;

};

#endif