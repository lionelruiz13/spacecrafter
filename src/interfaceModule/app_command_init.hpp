#ifndef APPCOMMANDINIT_HPP
#define APPCOMMANDINIT_HPP

#include"interfaceModule/app_command_interface.hpp"

class AppCommandInit {
public: 
    AppCommandInit();
    ~AppCommandInit();

    std::map<const std::string, SC_COMMAND> initialiseCommandsName();
    std::map<const std::string, FLAG_NAMES> initialiseFlagsName();
    std::map<const std::string, COLORCOMMAND_NAMES> initialiseColorCommand();
    std::map<const std::string, SCD_NAMES> initialiseSetCommand();

};

#endif