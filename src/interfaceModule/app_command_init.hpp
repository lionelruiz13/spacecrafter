#ifndef APPCOMMANDINIT_HPP
#define APPCOMMANDINIT_HPP

#include"interfaceModule/app_command_interface.hpp"

class AppCommandInit {
public: 
    AppCommandInit(AppCommandInterface* _app);
    ~AppCommandInit();

private: 
    AppCommandInterface *app = nullptr;
};

#endif