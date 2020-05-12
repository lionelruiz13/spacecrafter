#ifndef APPCOMMANDINIT_HPP
#define APPCOMMANDINIT_HPP

#include "interfaceModule/app_command_interface.hpp"
#include <map>
#include <list>

class AppCommandInit {
public: 
    AppCommandInit();
    ~AppCommandInit();

    void initialiseCommandsName(std::map<const std::string, SC_COMMAND> &m_commands, std::map<SC_COMMAND, const std::string> &m_commandsToString);
    void initialiseFlagsName(std::map<const std::string, FLAG_NAMES> &m_flags, std::map<FLAG_NAMES,const std::string> &m_flagsToString);
    void initialiseColorCommand(std::map<const std::string, COLORCOMMAND_NAMES> &m_color, std::map<COLORCOMMAND_NAMES, const std::string> &m_colorToString);
    void initialiseSetCommand(std::map<const std::string, SCD_NAMES> &m_set, std::map<SCD_NAMES, const std::string> &m_setToString);


    void searchSimilarCommand(const std::string& source) {
        this->searchNeighbour(source,commandList);
    }
    void searchSimilarFlag(const std::string& source) {
        this->searchNeighbour(source,flagList);
    }
    void searchSimilarColor(const std::string& source) {
        this->searchNeighbour(source,colorList);
    }
    void searchSimilarSet(const std::string& source) {
        this->searchNeighbour(source,setList);
    }

    //template<typename T> void searchNeighbour(const std::string &source, const std::map<std::string , T> &target);
private:
    void setObsoleteToken();
    bool isObsoleteToken(const std::string &name);
    void searchNeighbour(const std::string &source, const std::list<std::string> &target);
    //for conivence
    std::list<std::string> commandList;
    std::list<std::string> flagList;
    std::list<std::string> colorList;
    std::list<std::string> setList;
    std::list<std::string> obsoletList;
};

#endif