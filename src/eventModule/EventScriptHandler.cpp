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


#include "EventScriptHandler.hpp"
#include "scriptModule/script_interface.hpp"
#include "eventModule/EventScript.hpp"

void EventScriptHandler::handle(const Event* e, Executor *executor)
{
	ScriptEvent * event = (ScriptEvent *)e;
	scriptInterface->playScript(event->getFileName());
}
