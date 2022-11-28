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

#include "bodyModule/solarsystem_display.hpp"
#include "bodyModule/ssystem_iterator.hpp"
#include "bodyModule/solarsystem.hpp"
#include "bodyModule/halo.hpp"
#include "navModule/navigator.hpp"
#include "navModule/observer.hpp"
#include "coreModule/projector.hpp"
#include "tools/context.hpp"
#include "tools/draw_helper.hpp"
#include "EntityCore/Core/FrameMgr.hpp"
#include "EntityCore/Resource/Pipeline.hpp"

SolarSystemDisplay::SolarSystemDisplay(ProtoSystem * _ssystem)
{
    ssystem = _ssystem;
}

void SolarSystemDisplay::computePreDraw(const Projector * prj, const Navigator * nav)
{
	if (!getFlagShow())
		return; // 0;

    // Compute each Body distance to the observer and
    // sort all body from the furthest to the closest to the observer
    ssystem->computeDraw(prj, nav);

	// Determine optimal depth buffer buckets for drawing the scene
	// This is similar to Celestia, but instead of using ranges within one depth
	// buffer we just clear and reuse the entire depth buffer for each bucket.
	listBuckets.clear();
	depthBucket db {};

    const auto _end = ssystem->endSorted();
	for (auto it = ssystem->beginSorted(); it != _end; ++it) {
        auto &body = **it;
        if (!body.isVisibleOnScreen()) // Only reserve a bucket for visible body
            continue;

		double bounding = body.getBoundingRadius() * 1.01;

		if (bounding <= 0) // Check if it has a bounding
            continue;

        double dist = body.getEarthEquPos(nav).length();  // AU
		double znear = dist - bounding;
		double zfar  = dist + bounding;

		if (znear < 0.00000001) // The camera is in the bucket !
			znear = 0.00000001;

		// see if overlaps previous bucket
		// TODO check that buffer isn't too deep
		if (db.znear < zfar) {
			// merge with last bucket

			//cout << "merged buckets " << (*iter)->getEnglishName() << " " << znear << " " << zfar << " with " << lastNear << " " << lastFar << endl;
			if (db.znear > znear || db.znear == 0) {
				// Artificial planets may cover real planets, for example
				db.znear = znear;
			}

			if (db.zfar < zfar) {
				db.zfar = zfar;
			}
		} else {
			// create a new bucket
			//cout << "New bucket: " << (*iter)->getEnglishName() << znear << " zfar: " << zfar << endl;
            listBuckets.push_back(db);
			db.znear = znear;
			db.zfar  = zfar;
		}
	}
    if (db.znear)
        listBuckets.push_back(db);
}

// Draw all the elements of the solar system
// We are supposed to be in heliocentric coordinate
void SolarSystemDisplay::draw(Projector * prj, const Navigator * nav, const Observer* observatory, const ToneReproductor* eye, /*bool flag_point,*/ bool drawHomePlanet)
{
	if (!getFlagShow())
		return; // 0;

	Halo::beginDraw();
    auto &context = *Context::instance;
    context.helper->nextDraw(PASS_MULTISAMPLE_DEPTH);

    // Save clipping planes
	depthBucket backup;
	prj->getClippingPlanes(&backup.znear, &backup.zfar);

    int nBuckets = listBuckets.size();
	auto dbiter = (nBuckets) ? listBuckets.data() : &backup;
    // Draw the elements
    prj->setClippingPlanes(dbiter->znear, dbiter->zfar);

	// economize performance by not clearing depth buffer for each bucket... good?
	//	cout << "\n\nNew depth rendering loop\n";
	bool depthTest = true;  // small objects don't use depth test for efficiency
    bool newBucket = true;
	double dist;

    //! Computing orbit bucket need drawHomePlanet
    depthBucket orbitBucket{.znear = 1e10, .zfar = 0};

    const auto _end = ssystem->endSorted();
	for (auto it = ssystem->beginSorted(); it != _end; ++it) {
        auto &body = **it;
		dist = body.getDistance();
        if (body.needOrbitDepth()) {
            // Add this body to the orbit depth bucket
            const double bounding = body.getBoundingRadius() * 1.01;
            if (orbitBucket.znear > dist - bounding)
                orbitBucket.znear = dist - bounding;
            if (orbitBucket.zfar < dist + bounding)
                orbitBucket.zfar = dist + bounding;
        }
		if (dist < dbiter->znear) {
			//~ std::cout << "Change of bucket for " << (*iter)->englishName << " which has as parent " << (*iter)->body->getParent()->getEnglishName() << std::endl;
			//~ std::cout << "Change of bucket for " << (*iter)->englishName << std::endl;

			// potentially use the next depth bucket
			if (--nBuckets > 0) {
                // Select the next depth bucket
                ++dbiter;
                newBucket = true;
				prj->setClippingPlanes(dbiter->znear, dbiter->zfar);
			}
		}
		if (dist > dbiter->zfar || dist < dbiter->znear) {
			// don't use depth test (outside buckets)
			// std::cout << "Outside bucket for " << it->current()->body->getEnglishName() << std::endl;
			if (depthTest)
				prj->setClippingPlanes(backup.znear, backup.zfar);
			depthTest = false;
		} else {
			if (!depthTest)
				prj->setClippingPlanes(dbiter->znear, dbiter->zfar);
			depthTest = true;
			//~ std::cout << "inside bucket for " << (*iter)->englishName << std::endl;
		}
        if (newBucket && depthTest) {
            newBucket = !body.drawGL(prj, nav, observatory, eye, depthTest, drawHomePlanet, true);
            // if (!newBucket)
            //     std::cout << "Bucket cleared by " << it->current()->body->getEnglishName() << ", as usual.\n";
        } else {
            body.drawGL(prj, nav, observatory, eye, depthTest, drawHomePlanet, false);
                // std::cout << "Draw outside bucket for " << it->current()->body->getEnglishName() << ", is it what you expect ?\n";
        }
	}
	Halo::endDraw();

    // We should configure the clipping planes to include the relevant bodies for orbit tracing
    if (orbitBucket.zfar == 0)
        orbitBucket = backup;
    prj->setClippingPlanes(std::max(orbitBucket.znear, 0.00000001), orbitBucket.zfar);

    // Draw the orbits
    if (cmds[0] == -1) {
        cmds[0] = context.frame[0]->create(2);
        cmds[1] = context.frame[1]->create(2);
        cmds[2] = context.frame[2]->create(2);
    }
    auto &frame = *context.frame[context.frameIdx];
    VkCommandBuffer cmdBodyDepth = frame.begin(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
    VkCommandBuffer cmdOrbit = frame.begin(cmds[context.frameIdx]+1, PASS_MULTISAMPLE_DEPTH);
    { // preDrawOrbit
        const VkDeviceSize zero = 0;
        BodyShader::getShaderDepthTrace()->pipeline->bind(cmdBodyDepth);
        vkCmdBindVertexBuffers(cmdBodyDepth, 0, 1, &context.ojmBufferMgr->getBuffer(), &zero);
        vkCmdBindIndexBuffer(cmdBodyDepth, context.indexBufferMgr->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        VkClearAttachment clearAttachment {VK_IMAGE_ASPECT_DEPTH_BIT, 0, {.depthStencil={1.f,0}}};
        VkClearRect clearRect {VulkanMgr::instance->getScreenRect(), 0, 1};
        vkCmdClearAttachments(cmdBodyDepth, 1, &clearAttachment, 1, &clearRect);
    }
    if (drawHomePlanet) {
        std::for_each(ssystem->beginSorted(), ssystem->endSorted(), [cmdBodyDepth, cmdOrbit, observatory, nav, prj](Body *body) {
            body->drawOrbit(cmdBodyDepth, cmdOrbit, observatory, nav, prj);
        });
    } else {
        std::for_each(ssystem->beginSorted(), ssystem->endSorted(), [cmdBodyDepth, cmdOrbit, observatory, nav, prj](Body *body) {
            if (!observatory->isOnBody(body))
                body->drawOrbit(cmdBodyDepth, cmdOrbit, observatory, nav, prj);
        });
    }
    frame.compile(cmdBodyDepth);
    frame.compile(cmdOrbit);
    frame.toExecute(cmds[context.frameIdx], PASS_MULTISAMPLE_DEPTH);
    frame.toExecute(cmds[context.frameIdx]+1, PASS_MULTISAMPLE_DEPTH);

	prj->setClippingPlanes(backup.znear, backup.zfar);  // Restore old clipping planes
}

// Compute the position for every elements of the solar system.
// The order is not important since the position is computed relatively to the mother body
void SolarSystemDisplay::computePositions(double date,const Observer *obs)
{
	if (flag_light_travel_time) {
		const Vec3d home_pos(obs->getHeliocentricPosition(date));
		for (auto &v : *ssystem) {
			const double light_speed_correction =
			    (v.second.body->get_heliocentric_ecliptic_pos()-home_pos).length()
			    * (149597870000.0 / (299792458.0 * 86400));
			v.second.body->compute_position(date-light_speed_correction);
		}
	} else {
		for (auto &v : *ssystem) {
			v.second.body->compute_position(date);
		}
	}

	computeTransMatrices(date, obs);
}


// Compute the transformation matrix for every elements of the solar system.
// The elements have to be ordered hierarchically, eg. it's important to compute earth before moon.
void SolarSystemDisplay::computeTransMatrices(double date,const Observer * obs)
{
	if (flag_light_travel_time) {
		const Vec3d home_pos(obs->getHeliocentricPosition(date));
		for (auto &v : *ssystem) {
			const double light_speed_correction =
			    (v.second.body->get_heliocentric_ecliptic_pos()-home_pos).length()
			    * (149597870000.0 / (299792458.0 * 86400));
			v.second.body->compute_trans_matrix(date-light_speed_correction);
		}
	} else {
		for (auto &v : *ssystem) {
			v.second.body->compute_trans_matrix(date);
		}
	}
}
