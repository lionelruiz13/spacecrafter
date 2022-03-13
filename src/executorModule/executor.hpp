/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
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
 * Spacecrafter is a free open project of of LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

// Class defining which drawing and update functions must be used

#ifndef _EXECUTOR_
#define _EXECUTOR_

#include <memory>

#include "coreModule/core.hpp"
#include "executorModule/executorModule.hpp"
#include "executorModule/solarSystemModule.hpp"
#include "executorModule/stellarSystemModule.hpp"
#include "executorModule/inGalaxyModule.hpp"
#include "executorModule/inUniverseModule.hpp"
#include "executorModule/inPauseModule.hpp"

/**
 * \file executor.hpp
 * \brief Update / Draw functions mode handling
 * \author Jérémy Calvo
 * \version 1
 *
 * \class Executor
 *
 * \brief Chooses the right draw / update function depending on current mode
 * 
 * Allows to change mode manually or automatically depending on the altitude.
 *
*/
class Executor {
public:
    Executor(std::shared_ptr<Core> _core, Observer *_observer);

    void draw(int delta_time);
    void update(int delta_time);

    // Update mode depending on observer's altitude
	void updateMode(double altitude);

    void switchMode(const std::string &mode);
	
    void onAltitudeChange(double value) {
		std::cout << "Modification altitude reçue "<< value << std::endl;
		core->setBodyDecor();
        updateMode(observer->getAltitude());
	}

    MODULE getExecutorModule() {
        return currentMode->getExecutorModule();
    }

private:
    std::shared_ptr<Core> core;
    Observer *observer;

    std::unique_ptr<SolarSystemModule> ssystemModule;
    std::unique_ptr<StellarSystemModule> stellarSystemModule;
    std::unique_ptr<InGalaxyModule> inGalaxyModule;
    std::unique_ptr<InUniverseModule> inUniverseModule;
    //std::unique_ptr<InPauseModule> inPauseModule;
    ExecutorModule *currentMode;
};

#endif
