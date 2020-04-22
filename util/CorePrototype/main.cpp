#include <iostream>
#include <unistd.h>
#include "app.hpp"


int main() {
    App* app = new App();

	for (int i=0;i<3; i++) {
		std::cout << std::endl << " --- DRAW --- " << std::endl;
		app->draw();
		std::cout << std::endl << " --- UPDATE --- " << std::endl;
		app->update();
		sleep(1);
	}
    return 0;
}
