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


#include "EventSaveScreenHandler.hpp"
#include "appModule/save_screen_interface.hpp"
#include "eventModule/EventSaveScreen.hpp"


void EventSaveScreenHandler::handle(const Event* e)
{
    SaveScreenEvent * event = (SaveScreenEvent *)e;
    switch(event->getOrder()) {
        case SAVESCREEN_ORDER::START_VIDEO : 
            saveScreenInterface->startVideo();
            break;   
        case SAVESCREEN_ORDER::STOP_VIDEO :
            saveScreenInterface->stopVideo();
            break;
        case SAVESCREEN_ORDER::TAKE_SCREENSHOT :
            saveScreenInterface->takeScreenShot();
            break;
        case SAVESCREEN_ORDER::TOGGLE_VIDEO :
            saveScreenInterface->takeVideoShot();
      // default: break;
    }
}
