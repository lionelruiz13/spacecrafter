#include <iostream>
#include"app.hpp"

int main() {
    App* app = new App();

    app->start_main_loop();

    return 0;
}