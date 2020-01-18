#ifndef OBSERVER_HPP
#define OBSERVER_HPP


#include "event_manager.hpp"

class Observer{
public:

    void setAltitude(double alt){
        altitude = alt;
        Event event(Event::Altitude_Changed, alt);
        EventManager::getInstance()->queue(event);
    }

private:
    double altitude;
};


#endif
