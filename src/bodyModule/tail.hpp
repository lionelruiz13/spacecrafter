/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2022 Immersive Adventure
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

#ifndef TAIL_HPP_
#define TAIL_HPP_

#include <memory>
#include <vulkan/vulkan.h>
#include "tools/vecmath.hpp"
struct TailContext;
class SmallBody;
class Navigator;

class Tail {
public:
    Tail(float deltaTraceJD = 30, float ejectionForce = 0.5, const Vec3f &coefRadius = {-2, 1, 2}, const Vec3f &color = {0.5, 0.5, 0.5});
    ~Tail();

    void draw(const Navigator *nav, SmallBody *body, const Vec3f &eye_planet, const Vec3f &eye_sun, float radius, float JD);
    // void draw(const Vec3f &eyePosition, const Vec3f &eyeDirection, const Vec3f &eyeCorrection, const Vec3f &coefRadius);

    static void beginDraw(float fov);
    static void drawBatch(VkCommandBuffer cmd);
    static void endDraw();
    static bool shouldDraw();
protected:
    float deltaTraceJD = 30;
    float ejectionForce = 0.5;
    Vec3f coefRadius = {-2, 1, 2};
    Vec3f color = {0.5, 0.5, 0.5};
private:
    static std::weak_ptr<TailContext> globalRef;
    static TailContext *global;
    float lastJD = 0;
    Vec3f cachedExpansionInitial = {};
    Vec3f cachedExpansionCorrection = {};
    Vec3f cachedCoefRadius = {};
    std::shared_ptr<TailContext> shared;
};

#endif /* end of include guard: TAIL_HPP_ */
