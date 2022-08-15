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

SolarSystemDisplay::SolarSystemDisplay(ProtoSystem * _ssystem)
{
    ssystem = _ssystem;
}

void SolarSystemDisplay::computePreDraw(const Projector * prj, const Navigator * nav)
{
	if (!getFlagShow())
		return; // 0;

	// Compute each Body distance to the observer
	Vec3d obs_helio_pos = nav->getObserverHelioPos();
	//	cout << "obs: " << obs_helio_pos << endl;
	for (auto it = ssystem->createIteratorVector(); !it->end(); (*it)++){
		it->current()->body->compute_distance(obs_helio_pos);
		it->current()->body->computeMagnitude(obs_helio_pos);
		it->current()->body->computeDraw(prj, nav);
	}

	// sort all body from the furthest to the closest to the observer
	sort(ssystem->begin(), ssystem->end(), [] (std::shared_ptr<ProtoSystem::BodyContainer> const b1, std::shared_ptr<ProtoSystem::BodyContainer> const b2) {
		return (b1->body->getDistance() > b2->body->getDistance());
	});

	// Determine optimal depth buffer buckets for drawing the scene
	// This is similar to Celestia, but instead of using ranges within one depth
	// buffer we just clear and reuse the entire depth buffer for each bucket.
	double znear, zfar;
	double lastNear = 0;
	double lastFar = 0;
	int nBuckets = 0;
	listBuckets.clear();
	depthBucket db;

	for (auto it = ssystem->createIteratorVector(); !it->end() ;(*it)++) {
		if ( it->current()->body->getTurnAround() == tACenter
		        // This will only work with natural planets
		        // and not some illustrative (huge) artificial planets for example
		        && it->current()->body->get_on_screen_bounding_size(prj, nav) > 3 ) {

			//~ std::cout << "Calcul de bucket pour " << (*iter)->englishName << std::endl;
			double dist = it->current()->body->getEarthEquPos(nav).length();  // AU
			double bounding = it->current()->body->getBoundingRadius() * 1.01;

			if ( bounding >= 0 ) {
				// this is not a hidden object

				znear = dist - bounding;
				zfar  = dist + bounding;

				if (znear < 0.001){
					znear = 0.00000001;
				}
				else{
					if (znear < 0.05) {
						znear *= 0.1;
					}
					else{
						if (znear < 0.5){
							znear *= 0.2;
						}
					}
				}

				// see if overlaps previous bucket
				// TODO check that buffer isn't too deep
				if ( nBuckets > 0 && zfar > lastNear ) {
					// merge with last bucket

					//cout << "merged buckets " << (*iter)->getEnglishName() << " " << znear << " " << zfar << " with " << lastNear << " " << lastFar << endl;
					db = listBuckets.back();

					if(znear < lastNear ) {
						// Artificial planets may cover real planets, for example
						lastNear = db.znear = znear;
					}

					if ( zfar > lastFar ) {
						lastFar = db.zfar = zfar;
					}

					listBuckets.pop_back();
					listBuckets.push_back(db);

				} else {

					// create a new bucket
					//cout << "New bucket: " << (*iter)->getEnglishName() << znear << " zfar: " << zfar << endl;
					lastNear = db.znear = znear;
					lastFar  = db.zfar  = zfar;
					nBuckets++;
					listBuckets.push_back( db );
				}
			}
		}
	}
}

// Draw all the elements of the solar system
// We are supposed to be in heliocentric coordinate
void SolarSystemDisplay::draw(Projector * prj, const Navigator * nav, const Observer* observatory, const ToneReproductor* eye, /*bool flag_point,*/ bool drawHomePlanet)
{
	if (!getFlagShow())
		return; // 0;

	Halo::beginDraw();
    Context::instance->helper->nextDraw(PASS_MULTISAMPLE_DEPTH);
	int nBuckets = listBuckets.size();

	std::list<depthBucket>::iterator dbiter;

	//~ cout << "***\n";
	//~ dbiter = listBuckets.begin();
	//~ while( dbiter != listBuckets.end() ) {
	//~ cout << (*dbiter).znear << " " << (*dbiter).zfar << endl;
	//~ dbiter++;
	//~ }
	//~ cout << "***\n";

	// Draw the elements
	double z_near, z_far;
	prj->getClippingPlanes(&z_near,&z_far); // Save clipping planes

	dbiter = listBuckets.begin();

	// clear depth buffer
	prj->setClippingPlanes((*dbiter).znear*.99, (*dbiter).zfar*1.01);
	//glClear(GL_DEPTH_BUFFER_BIT);
	// Renderer::clearDepthBuffer();

	//float depthRange = 1.0f/nBuckets;
	float currentBucket = nBuckets - 1;

	// economize performance by not clearing depth buffer for each bucket... good?
	//	cout << "\n\nNew depth rendering loop\n";
	bool depthTest = true;  // small objects don't use depth test for efficiency
    bool newBucket = true;
	double dist;

	for (auto it = ssystem->createIteratorVector(); !it->end(); (*it)++) {
        auto *bodyHandle = &it->current().get();
        if (!body) {
//            cLog::get()->write("Nullptr received as body in the system display", LOG_TYPE::L_WARNING);
            continue;
        }
		dist = bodyHandle->body->getEarthEquPos(nav).length();
		if (dist < (*dbiter).znear ) {
			//~ std::cout << "Changement de bucket pour " << (*iter)->englishName << " qui a pour parent " << (*iter)->body->getParent()->getEnglishName() << std::endl;
			//~ std::cout << "Changement de bucket pour " << (*iter)->englishName << std::endl;

			// potentially use the next depth bucket
			dbiter++;

			if (dbiter == listBuckets.end() ) {
				dbiter--;
				// now closer than the first depth buffer
			} else {
				currentBucket--;

				// TODO: evaluate performance tradeoff???
				// glDepthRange(currentBucket*depthRange, (currentBucket+1)*depthRange);
				// if (needClearDepthBuffer) {
				// 	// glClear(GL_DEPTH_BUFFER_BIT);
				// 	Renderer::clearDepthBuffer();
				// 	needClearDepthBuffer = false;
				// }
                newBucket = true;

				// get ready to start using
				prj->setClippingPlanes((*dbiter).znear*.99, (*dbiter).zfar*1.01);
			}
		}
		if (dist > (*dbiter).zfar || dist < (*dbiter).znear) {
			// don't use depth test (outside buckets)
			// std::cout << "Outside bucket pour " << it->current()->body->getEnglishName() << std::endl;
			if ( depthTest )
				prj->setClippingPlanes(z_near, z_far);
			depthTest = false;

		} else {
			if (!depthTest)
				prj->setClippingPlanes((*dbiter).znear*.99, (*dbiter).zfar*1.01);

			depthTest = true;
			//~ std::cout << "inside bucket pour " << (*iter)->englishName << std::endl;
		}
        if (newBucket && depthTest) {
            newBucket = !bodyHandle->body->drawGL(prj, nav, observatory, eye, depthTest, drawHomePlanet, true);
            // if (!newBucket)
            //     std::cout << "Bucket cleared by " << it->current()->body->getEnglishName() << ", as usual.\n";
        } else {
            bodyHandle->body->drawGL(prj, nav, observatory, eye, depthTest, drawHomePlanet, false);
                // std::cout << "Draw outside bucket for " << it->current()->body->getEnglishName() << ", is it what you expect ?\n";
        }
	}
	Halo::endDraw();
	prj->setClippingPlanes(z_near,z_far);  // Restore old clipping planes
}

// Compute the position for every elements of the solar system.
// The order is not important since the position is computed relatively to the mother body
void SolarSystemDisplay::computePositions(double date,const Observer *obs)
{
	if (flag_light_travel_time) {
		const Vec3d home_pos(obs->getHeliocentricPosition(date));
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			const double light_speed_correction =
			    (it->current()->second->body->get_heliocentric_ecliptic_pos()-home_pos).length()
			    * (149597870000.0 / (299792458.0 * 86400));
			it->current()->second->body->compute_position(date-light_speed_correction);
		}
	} else {
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			it->current()->second->body->compute_position(date);
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
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			const double light_speed_correction =
			    (it->current()->second->body->get_heliocentric_ecliptic_pos()-home_pos).length()
			    * (149597870000.0 / (299792458.0 * 86400));
			it->current()->second->body->compute_trans_matrix(date-light_speed_correction);
		}
	} else {
		for(auto it = ssystem->createIterator(); !it->end(); (*it)++){
			it->current()->second->body->compute_trans_matrix(date);
		}
	}
}
