/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 Elitit-40
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


#ifndef EVENT_HPP
#define EVENT_HPP

#include <string>
#include <map>
#include <iostream>

#ifdef WIN32
// Win32 define E_NOT_SET, remove it's definition to avoid conflict with event flag
#undef E_NOT_SET
#endif

class Event{

public :
    enum Event_Type : char {
        E_NOT_SET = 0,
        E_SCRIPT,
        E_COMMAND,
        E_SCREEN_FADER,
        E_SCREEN_FADER_INTERLUDE,
        E_FLAG,
        E_SAVESCREEN,
        E_FPS,
        E_CHANGE_ALTITUDE,
        E_CHANGE_OBSERVER,
        E_VIDEO
        //....
    };

    Event(){
        type = E_NOT_SET;
    }

    virtual ~Event(){};

    Event(Event_Type _type){
		type=_type;
	}

    Event_Type getEventType() const {
        return type;
    }

    static std::map<Event_Type, std::string> eventTypeToString;

    friend std::ostream& operator<< (std::ostream & os, const Event& e){
        os << e.toString() << std::endl;
        return os;
    }

    virtual std::string toString() const {
        return Event::eventTypeToString[type];
    }

protected :
    Event_Type type;
};


#endif
