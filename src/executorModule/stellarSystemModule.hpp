/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2021 Jérémy Calvo
 * Copyright (C) 2022 Calvin Ruiz
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

#ifndef _STELLARSYSTEM_MODULE_
#define _STELLARSYSTEM_MODULE_

#include "executorModule.hpp"
#include "coreModule/core.hpp"
#include "mediaModule/media.hpp"
#include "EntityCore/Tools/SafeQueue.hpp"
#include <thread>

class StellarSystemModule : public ExecutorModule {
public:

    StellarSystemModule(std::shared_ptr<Core> _core, Observer *_observer);
    ~StellarSystemModule();

    virtual void onEnter() override;
	virtual void onExit() override;
	virtual void update(int delta_time) override;
	virtual void draw(int delta_time) override;
	virtual bool testValidAltitude(double altitude) override;

private:
    // Start async update
    void asyncUpdateBegin(std::pair<Vec3d, Vec3d> data);
    // Ensure async update has completed before continue
    void asyncUpdateEnd();
    void asyncUpdateLoop();
    bool asyncWorkState = false; // used by asycnUpdateBegin and asyncUpdateEnd
    std::shared_ptr<Core> core;
    Observer *observer;
    Vec3d center;
    std::thread thread;
    WorkQueue<std::pair<Vec3d, Vec3d>, 3> threadQueue;
};

#endif
