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


#include "EventFpsHandler.hpp"
#include "appModule/fps.hpp"
#include "eventModule/FpsEvent.hpp"


void EventFpsHandler::handle(const Event* e)
{
    FpsEvent * event = (FpsEvent *)e;
    switch(event->getOrder()) {
        case FPS_ORDER::LOW_FPS :
            clock->fixScriptFps(); break;

        case FPS_ORDER::HIGH_FPS :
            clock->fixMaxFps(); break;

        case FPS_ORDER::AFTER_ONE_SECOND :
            clock->afterOneSecond(); break;

        default: break;
    }
}
