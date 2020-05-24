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

/**
* \file AppCommandEvent.hpp
* \brief Create Event to AppCommandInterface
* \author Elitit
* \version 1
*/

#ifndef APP_COMMAND_EVENT_HPP
#define APP_COMMAND_EVENT_HPP

#include <string>
#include <sstream>
#include "eventModule/event.hpp"
#include "interfaceModule/base_command_interface.hpp"

/**
* \class CommandEvent
*
* \brief Create a new generic command for AppCommandInterface
*/
class CommandEvent : public Event {
public:
    //! constructor who takes the command as parameter
    CommandEvent(const std::string& _commandLine) : Event(E_COMMAND) {
        commandLine = _commandLine;
    }
    ~CommandEvent(){};

    //! returns the character string representing the order to analyze
    std::string getCommandLine() {
        return commandLine;
    }

    //! returns the type of Event and the information conveyed by the event
    virtual std::string toString() const {
        std::ostringstream os;
        os << Event::toString() << " CommandName : " << commandLine << std::endl;
        return os.str();
    }
  
private:
    std::string commandLine;
};

/**
* \class FlagEvent
*
* \brief Create a new flag command, already parsed, for AppCommandInterface
*/
class FlagEvent : public Event {
public:
    FlagEvent(FLAG_NAMES _flagName, FLAG_VALUES _flagValue) : Event(E_FLAG) {
        flagName = _flagName;
        flagValue = _flagValue;
    }
    ~FlagEvent(){};

    FLAG_NAMES getFlagName() const {
        return flagName;
    }

    FLAG_VALUES getFlagValue() const {
        return flagValue;
    }

    virtual std::string toString() const {
        std::ostringstream os;
        os << Event::toString() << " FlagEvent " << std::endl;
        return os.str();
}
    
private:
    FLAG_NAMES flagName;
    FLAG_VALUES flagValue;
};

#endif