#ifndef ALTI_HPP
#define ALTI_HPP


#include "event_manager.hpp"

class Altimetre{
public:

    void set(bool v){
        alti = v;
        Event event(Event::E_Altimetre, v);
        EventManager::getInstance()->queue(event);
    }

private:
    double alti;
};


#endif
