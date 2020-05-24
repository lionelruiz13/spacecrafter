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


#ifndef FLAG_EVENT_HPP
#define FLAG_EVENT_HPP

#include <string>
#include <sstream>
#include "eventModule/event.hpp"
#include "interfaceModule/base_command_interface.hpp"

class FlagEvent : public Event {
public:
    FlagEvent(FLAG_NAMES _flagName, FLAG_VALUES _flagValue/*, const std::string _commandline*/) : Event(E_FLAG) {
        flagName = _flagName;
        flagValue = _flagValue;
        // commandLine = _commandline;
    }
    ~FlagEvent(){};

    FLAG_NAMES getFlagName() const {
        return flagName;
    }

    FLAG_VALUES getFlagValue() const {
        return flagValue;
    }

    // std::string getTranslation() const {
    //     return commandLine;
    // }

    virtual std::string toString() const {
        std::ostringstream os;
        os << Event::toString() << " FlagEvent " << std::endl;
        return os.str();
}
    
private:
    FLAG_NAMES flagName;
    FLAG_VALUES flagValue;
    // std::string commandLine;
};

#endif //FLAG_EVENT