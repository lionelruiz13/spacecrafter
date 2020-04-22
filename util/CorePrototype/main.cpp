#include <iostream>
#include"app.hpp"

#include<iostream>

int main() {
    App* app = new App();

    std::cout << " --- DRAW --- " << std::endl;
    app->draw();
    std::cout << std::endl << " --- UPDATE --- " << std::endl;
    app->update();

    return 0;
}