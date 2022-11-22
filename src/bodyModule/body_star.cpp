/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017 Immersive Adventure
 * Copyright (C) 2017-2020 AssociationSirius
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

#include "body_star.hpp"
#include "body_color.hpp"
#include "navModule/observer.hpp"
#include "tools/context.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "tools/draw_helper.hpp"
#include "bodyModule/axis.hpp"
#include "bodyModule/hints.hpp"
#include "bodyModule/halo.hpp"

BodyStar::BodyStar(std::shared_ptr<Body> parent,
        const std::string& englishName,
        bool flagHalo,
        double radius,
        double oblateness,
        std::unique_ptr<BodyColor> _myColor,
        float _sol_local_day,
        float albedo,
        std::unique_ptr<Orbit> orbit,
        bool close_orbit,
        ObjL* _currentObj,
        double orbit_bounding_radius,
        const BodyTexture &_bodyTexture) :
    Sun(parent,
        englishName,
        flagHalo,
        radius,
        oblateness,
        std::move(_myColor),
        _sol_local_day,
        albedo,
        std::move(orbit),
        close_orbit,
        _currentObj,
        orbit_bounding_radius,
        _bodyTexture,
        STAR)
{
    starViewer = std::make_unique<StarViewer>(myColor->getHalo(), radius, _currentObj);
}

BodyStar::~BodyStar()
{
}

bool BodyStar::drawGL(Projector* prj, const Navigator* nav, const Observer* observatory, const ToneReproductor* eye, bool depthTest, bool drawHomePlanet, bool needClearDepthBuffer)
{
	bool drawn = false;

	//on ne dessine pas une planete sur laquel on se trouve
	if (!drawHomePlanet && observatory->isOnBody(this)) {
		return drawn;
	}

	hints->drawHints(nav, prj);

	if (isVisibleOnScreen()) {  // huge improvement in performance
        Context &context = *Context::instance;
        FrameMgr &frame = *context.frame[context.frameIdx];
        if (cmds[context.frameIdx] == -1) {
            cmds[context.frameIdx] = frame.create(1);
            frame.setName(cmds[context.frameIdx], englishName + " " + std::to_string(context.frameIdx));
        }
        VkCommandBuffer cmd = frame.begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
        if (needClearDepthBuffer) {
            VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
            VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
            vkCmdClearAttachments(cmd, 1, &clearAttachment, 1, &clearRect);
        }
        if (screen_sz > 3) {
            context.helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
            Halo::nextDraw(cmd);
        }
        if (depthTest)
            axis->drawAxis(cmd, prj, mat);
        starViewer->setRadius(radius);
        starViewer->draw(nav, prj, mat, screen_sz);
        frame.compile(cmd);
        frame.toExecute(cmd, PASS_MULTISAMPLE_DEPTH);
        drawn = true;
	} else if (isVisible && tex_big_halo)
        drawBigHalo(nav, prj, eye);

	return drawn;
}
