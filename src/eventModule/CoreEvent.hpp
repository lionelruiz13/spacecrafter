/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2020 Elitit-40
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */


#ifndef CORE_EVENT_HPP
#define CORE_EVENT_HPP

#include <string>
#include <sstream>
#include "eventModule/event.hpp"

class AltitudeEvent : public Event {
public:
    AltitudeEvent(double _altitude) : Event(E_CHANGE_ALTITUDE) {
        altitude = _altitude;
    }
    ~AltitudeEvent(){};

    double getAltitude() {
        return altitude;
    }

    virtual std::string toString() const {
        std::ostringstream os;
        os << Event::toString() << " altitude changed to : " << altitude << std::endl;
        return os.str();
}
private:
    double altitude;
};


class ObserverEvent : public Event {
public:
    ObserverEvent(std::string _newObserver) : Event(E_CHANGE_OBSERVER) {
        newObserver = _newObserver;
    }
    ~ObserverEvent(){};

    const std::string& getNewObserver() {
        return newObserver;
    }

    virtual std::string toString() const {
        std::ostringstream os;
        os << Event::toString() << " observer changed to : " << newObserver << std::endl;
        return os.str();
}
private:
    std::string newObserver;
};

#endif