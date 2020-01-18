/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2018 Elitith-40
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

class Event{

public :
    enum Event_Type : char { 
        E_NOT_SET = 0,
        E_SCRIPT = 1,
        E_COMMAND = 2,
        E_SCREEN_FADER = 3,
        E_SCREEN_FADER_INTERLUDE  = 4,
        E_FLAG = 5,
        E_SAVESCREEN = 6,
        E_FPS
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
