#ifndef CORE_HPP
#define CORE_HPP

#include<iostream>

class Core {

public:
	Core();
	~Core();

    void init();

    void update();

	void draw();

    void setFlagSelectedObjectPointer();
    void setFlagTracking();
    void dragView();
    void unSelect();

private:

};

#endif