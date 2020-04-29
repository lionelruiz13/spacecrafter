#ifndef APPCOMMANDINIT_HPP
#define APPCOMMANDINIT_HPP

#include"interfaceModule/app_command_interface.hpp"

class CoreLink;

class AppCommandInit {
public: 
    AppCommandInit(AppCommandInterface* _app, CoreLink* _coreLink,  Core * core );
    ~AppCommandInit();

    void initialiseCommandsName(std::map<const std::string, SC_COMMAND> &m_commands);
    void initialiseFlagsName(std::map<const std::string, FLAG_NAMES> &m_flags);
    void initialiseColorCommand(std::map<const std::string, COLORCOMMAND_NAMES> &m_color);
    void initialiseSetCommand(std::map<const std::string, SCD_NAMES> &m_appcommand);

    void initialiseSetFlag(std::map<FLAG_NAMES, AppCommandInterface::stFct> &m_setFlag);

private:
    AppCommandInterface* appCommandInterface=nullptr;
    Core * stcore = nullptr;
    CoreLink* coreLink=nullptr; 

};

#endif