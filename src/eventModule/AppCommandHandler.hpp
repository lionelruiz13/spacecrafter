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
* \file AppCommandHandler.hpp
* \brief Send Event to AppCommandInterface
* \author Elitit
* \version 1
*/

#ifndef APP_COMMAND_HANDLER_HPP
#define APP_COMMAND_HANDLER_HPP


#include "event_handler_canvas.hpp"
#include "event.hpp"

class AppCommandInterface;

/**
* \class EventCommandHandler
*
* \brief Interface for generic command event to AppCommandInterface 
*/
class EventCommandHandler : public EventHandlerCanvas {
public:
	EventCommandHandler(AppCommandInterface *_commander) {
		commander = _commander;
	}
	~EventCommandHandler() {
	}
    void handle(const Event* e, Executor *executor) override;

protected :
	AppCommandInterface* commander = nullptr;
};


/**
* \class EventFlagHandler
*
* \brief Interface for flag event to AppCommandInterface 
*/
class EventFlagHandler : public EventHandlerCanvas {
public:
	EventFlagHandler(AppCommandInterface *_commander) {
		commander = _commander;
	}
	~EventFlagHandler() {
	}
    void handle(const Event* e, Executor *executor) override;

protected :
	AppCommandInterface* commander = nullptr;
};

#endif