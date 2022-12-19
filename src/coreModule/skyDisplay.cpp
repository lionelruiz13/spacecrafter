/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2002 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
 * Copyright (C) 2013 of the LSS team
 * Copyright (C) 2014 of the LSS Team & Association Sirius
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

#include "bodyModule/body.hpp"
#include "coreModule/skyDisplay.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include <string>
#include "tools/log.hpp"

#include "tools/s_font.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "tools/translator.hpp"

#include "tools/context.hpp"
#include "EntityCore/EntityCore.hpp"

#define NB_MAX_POINTS 4194304

const float deg2rad = 3.1415926 / 180.; // Convert deg to radian
const float rad2deg = 180. / 3.1415926; // Converd radian to deg
const float grad2rad = 3.1415926 / 18.; // Convert grind pas to radian
const float pi_div_2 = 1.5707963;		// pi/2

// -------------------- SKYLINE_PERSONAL  ---------------------------------------------

s_font* SkyDisplay::skydisplay_font = nullptr;
std::unique_ptr<VertexArray> SkyDisplay::vertexModel;
Pipeline *SkyDisplay::pipeline;
PipelineLayout *SkyDisplay::layout;
Set *SkyDisplay::set;
int SkyDisplay::virtualFragID;
int SkyDisplay::virtualMatID;

SkyDisplay::SkyDisplay(PROJECTION_TYPE _ptype)
{
	ptype = _ptype;
	createLocalResources();
	switch (ptype) {
		case AL:
			proj_func = &Projector::projectLocal;
			break;
		case EQ:
			proj_func = &Projector::projectEarthEqu;
			break;
		default:
			proj_func = &Projector::projectLocal;
			break;
	}
}

SkyDisplay::~SkyDisplay()
{
}

void SkyDisplay::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	vertexModel = std::make_unique<VertexArray>(vkmgr);
	vertexModel->createBindingEntry(3 * sizeof(float));
	vertexModel->addInput(VK_FORMAT_R32G32B32_SFLOAT);
	layout = new PipelineLayout(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 0, 1, true);
	layout->setUniformLocation(VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1, true);
	layout->buildLayout();
	layout->build();
	pipeline = new Pipeline(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout);
	pipeline->setDepthStencilMode();
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);
	pipeline->bindVertex(*vertexModel);
	pipeline->bindShader("person.vert.spv");
	pipeline->bindShader("person.geom.spv");
	pipeline->bindShader("person.frag.spv");
	pipeline->build();
	set = new Set(vkmgr, *context.setMgr, layout);
	virtualMatID = set->bindVirtualUniform(context.uniformMgr->getBuffer(), 0, sizeof(Mat4f));
	virtualFragID = set->bindVirtualUniform(context.uniformMgr->getBuffer(), 1, sizeof(frag));
}

void SkyDisplay::destroySC_context()
{
	delete pipeline;
	delete layout;
	delete set;
}

void SkyDisplay::createLocalResources()
{
	Context &context = *Context::instance;

	uFrag = std::make_unique<SharedBuffer<frag>>(*context.uniformMgr);
	uMat = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
}

void SkyDisplay::clear()
{
	dataSky = (Vec3f *) Context::instance->transfer->beginPlanCopy(NB_MAX_POINTS * 3 * sizeof(float));
	dataSkySize = 0;
}

void SkyDisplay::build()
{
	Context &context = *Context::instance;

	vertex = vertexModel->createBuffer(0, m_dataSize, context.globalBuffer.get());
	set->setVirtualUniform(uMat->getOffset(), virtualMatID);
	set->setVirtualUniform(uFrag->getOffset(), virtualFragID);
	if (cmds[0] == VK_NULL_HANDLE) {
		context.cmdInfo.commandBufferCount = 3;
		vkAllocateCommandBuffers(VulkanMgr::instance->refDevice, &context.cmdInfo, cmds);
		for (int i = 0; i < 3; ++i) {
			VkCommandBuffer &cmd = cmds[i];
			context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
			pipeline->bind(cmd);
			layout->bindSets(cmd, {*context.uboSet, *set}, *set);
			vertex->bind(cmd);
			vkCmdDraw(cmd, m_dataSize, 1, 0, 0);
			context.frame[i]->compile(cmd);
			context.frame[i]->setName(cmd, "SkyDisplay");
		}
	} else {
		needRebuild[0] = true;
		needRebuild[1] = true;
		needRebuild[2] = true;
	}
}

VkCommandBuffer SkyDisplay::getCommand()
{
	Context &context = *Context::instance;
	VkCommandBuffer cmd = cmds[context.frameIdx];

	if (needRebuild[context.frameIdx]) {
		needRebuild[context.frameIdx] = false;
		context.frame[context.frameIdx]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
		pipeline->bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set}, *set);
		vertex->bind(cmd);
		vkCmdDraw(cmd, m_dataSize, 1, 0, 0);
		context.frame[context.frameIdx]->compile(cmd);
	}
	return cmd;
}

//a optimize
void SkyDisplay::draw_text(const Projector *prj, const Navigator *nav)
{
	for (int i = -9; i < 10; i++) {
		std::ostringstream oss;
		//creation of point positions in pt3,pt4
		Utility::spheToRect(aperson - 0.31415926, (i - 0.0001) * grad2rad, pt3);
		Utility::spheToRect(aperson - 0.31415926 + 0.01, (i - 0.0001) * grad2rad, pt4);
		//test if pt3,pt4 is displayable and transmit to pt1,pt2 its position
		if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
			double angle;
			const double dx = pt1[0] - pt2[0];
			const double dy = pt1[1] - pt2[1];
			const double dq = dx * dx + dy * dy;
			const double d = sqrt(dq);
			angle = acos((pt1[1] - pt2[1]) / (d + 0.000001));
			if (pt1[0] < pt2[0])
				angle *= -1;
			if (i == -9)
				angle += 3.1415926;
			Mat4f MVP = prj->getMatProjectionOrtho2D();
			//sequence of position transformations from the coordinates of pt1
			Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
			TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
			//oss << pt1[0] << " " << pt2[0] << pt1[1] << " " << pt2[1];
			oss << i * 10 << "°";
			skydisplay_font->print(2, -2, oss.str(), color, MVP * TRANSFO, 1);
			oss.clear();
		}
	}
}

void SkyDisplay::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (fader.isZero() || m_dataSize <= 0) {
		return;
	}
	uFrag->get().fader = fader;
	*uMat = (ptype == AL) ? prj->getMatLocalToEye() : prj->getMatEarthEquToEye();
	Context::instance->frame[Context::instance->frameIdx]->toExecute(getCommand(), PASS_MULTISAMPLE_DEPTH);
}

///////////////////////////////////////////////////////////////////////
//
//      Derived classes
//
//
////////////////////////////////////////////////////////////////////////

SkyPerson::SkyPerson(PROJECTION_TYPE ptype) : SkyDisplay(ptype)
{}

void SkyPerson::loadData(const std::string& filename)
{
	double alpha, delta;
	int nblines;
	Vec3f punts;
	clear();

	std::ifstream fichier(filename, std::ios::in);
	if (fichier) {
		fichier >> nblines;
		if (nblines > NB_MAX_POINTS)
			nblines = NB_MAX_POINTS;

		for (int i = 0; i < nblines; i++) {
			fichier >> alpha >> delta;
			Utility::spheToRect(alpha, delta, punts);
			*(dataSky++) = punts;
			++dataSkySize;
		}
		aperson = alpha;
		fichier.close();
	}

	//we load the points in a vbo
	if (m_dataSize != dataSkySize) {
		m_dataSize = dataSkySize;
		build();
	}
	Context::instance->transfer->endPlanCopy(vertex->get(), m_dataSize * 3 * sizeof(float));
}

void SkyPerson::loadString(const std::string& message)
{
	//we first get the 2 numbers and then we convert them...
    std::string delimiter = ";";
    float ftemp;
	std::string txt = message;
	// std::cout << message << std::endl;
    // Checks for the presence of a letter
    for(std::string::size_type i = 0; i < txt.length(); i++)
    {
        char c = txt[i];
        if(!(isdigit(c)||c==';'||c=='.')){ //check si le caractère est une lettre
			cLog::get()->write("Skyperson error loading dataStr, check dataStr", LOG_TYPE::L_WARNING);
			// std::cout << "   " << c << std::endl;
            txt.erase(i, 1);
        }
    }
	// std::cout << txt << std::endl;

    size_t pos = 0;
    std::string token;

	std::vector<float> dataTmp;
    // Decompose the string
    while ((pos = txt.find(delimiter)) != std::string::npos) {
        token = txt.substr(0, pos);
		// std::cout << "   " << token << " | " ;
	    // more error resistant function
		std::istringstream dstr( token );
		dstr >> ftemp;
		//ftemp = std::stof(token);

        dataTmp.push_back(ftemp);
        txt.erase(0, pos + delimiter.length());
		// std::cout << txt  << std::endl;
    }
	// we still check that the content is a multiple of 2
	// otherwise we delete the last values.
	if (dataTmp.size()%2!=0) {
		dataTmp.pop_back();
	 	cLog::get()->write("Skyperson loading incomplete data", LOG_TYPE::L_WARNING);
	}

	// std::cout << "dataTmp a " << dataTmp.size()  << std::endl;

	clear();
	Vec3f punts;
	for (auto it =dataTmp.begin(); it!=dataTmp.end(); it++) {
			Utility::spheToRect(*it, *++it, punts);
			// std::cout << punts[0] << "|"<< punts[1] << "|"<< punts[2] << std::endl;
			*(dataSky++) = punts;
			++dataSkySize;
	}

	// we load the points in a vbo
	if (m_dataSize != dataSkySize) {
		m_dataSize = dataSkySize;
		build();
	}
	Context::instance->transfer->endPlanCopy(vertex->get(), m_dataSize * 3 * sizeof(float));
}


SkyNautic::SkyNautic(PROJECTION_TYPE ptype) : SkyDisplay(ptype)
{}

void SkyNautic::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (fader.isZero() || m_dataSize <= 0)
		return;

	Vec3f punts;
	clear();
	float tick;
	double direction;

	// calculate alt az
	double tempDE, tempRA;

	//Vec3d equPos = selected_object.getEarthEquPos(nav);
	Utility::rectToSphe(&tempRA, &tempDE, equPos);
	direction = tempRA * rad2deg;
	if (ptype == AL) {
		// calculate alt az position
		Vec3d localPos = nav->earthEquToLocal(equPos);
		Utility::rectToSphe(&tempRA, &tempDE, localPos);
		tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
		if (tempRA > M_PI * 2)
			tempRA -= M_PI * 2;
		direction = (M_PI - tempRA) * rad2deg;
	}
	for (int j = -9; j < 9; j++) {
		Utility::spheToRect(direction * deg2rad, j * grad2rad, punts);
		*(dataSky++) = punts;
		++dataSkySize;

		Utility::spheToRect(direction * deg2rad, (j + 1) * grad2rad, punts);
		*(dataSky++) = punts;
		++dataSkySize;

		for (int i = 0; i < 10; i++) {
			if (i == 1)
				tick = 0.6 * 90 / (90 - (j * 10 + i));
			else if (i == 5)
				tick = 0.4 * 90 / (90 - (j * 10 + i));
			else
				tick = 0.2 * 90 / (90 - (j * 10 + i));
			Utility::spheToRect((direction - tick) * deg2rad, (j * 10 + i) * deg2rad, punts);
			*(dataSky++) = punts;
			++dataSkySize;

			Utility::spheToRect((direction + tick) * deg2rad, (j * 10 + i) * deg2rad, punts);
			*(dataSky++) = punts;
			++dataSkySize;
		}
	}
	aperson = (direction + tick) * deg2rad;

	//we load the points in a vbo
	if (m_dataSize != dataSkySize) {
		m_dataSize = dataSkySize;
		build();
	}
	Context::instance->transfer->endPlanCopy(vertex->get(), m_dataSize * 3 * sizeof(float));

	SkyDisplay::draw(prj,nav);

	draw_text(prj, nav);
}

SkyCoords::SkyCoords() : SkyDisplay(PROJECTION_TYPE::AL)
{
}

void SkyCoords::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldPos)
{
	if (fader.isZero() || m_dataSize <= 0)
		return;

	Vec4f colorT (color[0], color[1], color[2], fader);

	double tempDE, tempRA;
	float alt, az, aza, alta, ra, dec, mn;
	double fov = prj->getFov() / 360.f;
	Utility::rectToSphe(&tempRA, &tempDE, equPos);
	// calculate ra dec
	ra = tempRA * rad2deg;
	if (ra < 0)
		ra = 360. + ra;
	ra = ra * 24. / 360.;
	dec = tempDE * rad2deg;
	// calculate alt az position
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	az = (tempRA)*rad2deg;
	alt = tempDE * rad2deg;
	aza = (M_PI - tempRA - (0.1 * fov)) * rad2deg;
	// ALT
	alta = (tempDE - (0.05 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//sequence of position transformations from the coordinates of punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "alt:";
		if (alt < 0.) {
			alt = -alt;
			oss << "-";
		}
		else
			oss << "+";
		if (alt < 10.)
			oss << "0";
		oss << truncf(alt) << "°";
		mn = truncf((alt - truncf(alt)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
		oss.clear();
	}
	// AZ
	alta = tempDE * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//sequence of position transformations from punts coordinates
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "az :";
		if (az < 0.) {
			az = -az;
			oss << "-";
		}
		else
			oss << "+";
		if (az < 100.)
			oss << "0";
		if (az < 10.)
			oss << "0";
		oss << truncf(az) << "°";
		mn = truncf((az - truncf(az)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
		oss.clear();
	}
	// RA
	alta = (tempDE + (0.1 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//sequence of position transformations from the coordinates of punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "ra :";
		if (ra < 10.)
			oss << "0";
		oss << truncf(ra) << "h";
		mn = truncf((ra - truncf(ra)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "m";
		skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
		oss.clear();
	}
	// DEC
	alta = (tempDE + (0.05 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);

	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//sequence of position transformations from the coordinates of punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "dec:";
		if (dec < 0.) {
			dec = -dec;
			oss << "-";
		}
		else
			oss << "+";
		if (dec < 10.)
			oss << "0";
		oss << truncf(dec) << "°";
		mn = truncf((dec - truncf(dec)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
		oss.clear();
	}
}


SkyMouse::SkyMouse() : SkyDisplay(PROJECTION_TYPE::AL)
{}

void SkyMouse::draw(const Projector *prj, const Navigator *nav, Vec3d _equPos, Vec3d _oldPos)
{
	if (fader.isZero() || m_dataSize <= 0)
		return;

	Vec4f colorT (color[0], color[1], color[2], fader);
	int x,y;
	SDL_GetMouseState(&x,&y);
	Vec3d equPos = prj->getCursorPosEqu(x, y);


	double tempDE, tempRA;
	float alt, az, aza, alta, ra, dec, mn;
	double fov = prj->getFov() / 360.f;
	Utility::rectToSphe(&tempRA, &tempDE, equPos);
	// calculate ra dec
	ra = tempRA * rad2deg;
	if (ra < 0)
		ra = 360. + ra;
	ra = ra * 24. / 360.;
	dec = tempDE * rad2deg;
	// calculate alt az position
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	az = (tempRA)*rad2deg;
	alt = tempDE * rad2deg;
	aza = (M_PI - tempRA - (0.1 * fov)) * rad2deg;
	// ALT
	alta = (tempDE - (0.05 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//sequence of position transformations from the coordinates of punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "alt:";
		if (alt < 0.) {
			alt = -alt;
			oss << "-";
		}
		else
			oss << "+";
		if (alt < 10.)
			oss << "0";
		oss << truncf(alt) << "°";
		mn = truncf((alt - truncf(alt)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1, false);
		oss.clear();
	}
	// AZ
	alta = tempDE * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//sequence of position transformations from the coordinates of punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "az :";
		if (az < 0.) {
			az = -az;
			oss << "-";
		}
		else
			oss << "+";
		if (az < 100.)
			oss << "0";
		if (az < 10.)
			oss << "0";
		oss << truncf(az) << "°";
		mn = truncf((az - truncf(az)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
		oss.clear();
	}
	// RA
	alta = (tempDE + (0.1 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);
	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//sequence of position transformations from the coordinates of punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "ra :";
		if (ra < 10.)
			oss << "0";
		oss << truncf(ra) << "h";
		mn = truncf((ra - truncf(ra)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "m";
		skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
		oss.clear();
	}
	// DEC
	alta = (tempDE + (0.05 * fov * 2)) * rad2deg;
	Utility::spheToRect(aza * deg2rad, alta * deg2rad, pt3);
	Utility::spheToRect(aza * deg2rad + (0.001 * fov), alta * deg2rad, pt4);

	if (((prj->*proj_func)(pt3, pt1)) && ((prj->*proj_func)(pt4, pt2))) {
		double angle;
		const double dx = pt1[0] - pt2[0];
		const double dy = pt1[1] - pt2[1];
		const double dq = dx * dx + dy * dy;
		const double d = sqrt(dq);
		angle = acos((pt1[1] - pt2[1]) / (d + 0.000001 * fov));
		if (pt1[0] < pt2[0])
			angle *= -1;
		std::ostringstream oss;
		Mat4f MVP = prj->getMatProjectionOrtho2D();
		//sequence of position transformations from the coordinates of punts
		Mat4f TRANSFO = Mat4f::translation(Vec3f(pt1[0], pt1[1], 0));
		TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
		oss << "dec:";
		if (dec < 0.) {
			dec = -dec;
			oss << "-";
		}
		else
			oss << "+";
		if (dec < 10.)
			oss << "0";
		oss << truncf(dec) << "°";
		mn = truncf((dec - truncf(dec)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
		skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
		oss.clear();
	}
}

SkyAngDist::SkyAngDist() : SkyDisplay(PROJECTION_TYPE::AL)
{}

void SkyAngDist::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (fader.isZero() || m_dataSize <= 0) {
		return;
	}

	Vec4f colorT (color[0], color[1], color[2], fader);
	double tempDE, tempRA, azt, altt, alt1, alt2, az1, az2;
	// for Selected position
	// calculate alt az position
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	alt1 = tempDE;
	az1 = M_PI - tempRA;
	// end of calculate alt az position

	// for Old position
	// calculate alt az position
	localPos = nav->earthEquToLocal(oldEquPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	alt2 = tempDE;
	az2 = M_PI - tempRA;
	// end of calculate alt az position
	if ((az2 - az1) > M_PI)
		az1 += 2 * M_PI;
	if ((az1 - az2) > M_PI)
		az2 += 2 * M_PI;

	// Draw orthodromy
	clear();
	Utility::spheToRect(az1, alt1, pt1);
	int npoints = 21;
	float delta = (az1 - az2) / (npoints - 1);
	for (int i = 0; i < npoints; i++) {
		*(dataSky++) = pt1;
		++dataSkySize;

		azt = az1 - delta * i;
		if (az1-az2 != 0)
		  altt = atan(((tan(alt2) * sin(azt - az1)) / sin(az2 - az1)) + (tan(alt1) * sin(az2 - azt)) / sin(az2 - az1));
        else
		  altt = M_PI/2.;
		Utility::spheToRect(azt, altt, pt1);
		if (i == 12)
			pt5 = pt1;
		*(dataSky++) = pt1;
		++dataSkySize;
	}

	if (m_dataSize != dataSkySize) {
		m_dataSize = dataSkySize;
		build();
	}
	Context::instance->transfer->endPlanCopy(vertex->get(), m_dataSize * 3 * sizeof(float));

	SkyDisplay::draw(prj,nav);

	// Text
	float ang, mn, sec;
	ang = acos(sin(alt1) * sin(alt2) + cos(alt1) * cos(alt2) * cos(az2 - az1)) * rad2deg;
	Utility::spheToRect(az1, alt1, pt3);
	Utility::spheToRect(az2, alt2, pt4);

	(prj->*proj_func)(pt3, pt1);
	(prj->*proj_func)(pt4, pt2);
	double angle;
	const double dx = pt1[0] - pt2[0];
	const double dy = pt1[1] - pt2[1];
	const double dq = dx * dx + dy * dy;
	const double d = sqrt(dq);
	angle = acos((pt1[1] - pt2[1]) / (d + 0.000001));
	if (pt1[0] < pt2[0])
		angle *= -1;
	std::ostringstream oss;
	Mat4f MVP = prj->getMatProjectionOrtho2D();
	//localPos = nav->earthEquToLocal(pt5);
	(prj->*proj_func)(pt5, pt0);
	Mat4f TRANSFO = Mat4f::translation(Vec3f(pt0[0], pt0[1], 0));
	TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
	if (truncf(ang) >= 1) {
		oss << truncf(ang) << "°";
		mn = truncf((ang - truncf(ang)) * 60);
		if (mn < 10)
			oss << "0";
		oss << mn << "'";
	} else {
		oss << truncf(ang * 60) << "'";
		sec = truncf((ang * 60 - truncf(ang * 60)) * 60);
		if (sec < 10)
			oss << "0";
		oss << sec << "\"";
	}
	skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
	oss.clear();
}

SkyLoxodromy::SkyLoxodromy() : SkyDisplay(PROJECTION_TYPE::EQ)
{}

void SkyLoxodromy::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (fader.isZero() || (equPos == oldEquPos)) {
		return;
	}
	Vec4f colorT (color[0], color[1], color[2], fader);
	double de1, ra1, de2, ra2, dem, ram;
	// for Old position
	Utility::rectToSphe(&ra1, &de1, oldEquPos);
	// for Selected position
	Utility::rectToSphe(&ra2, &de2, equPos);
	if ((ra2 - ra1) > M_PI)
		ra1 += 2 * M_PI;
	if ((ra1 - ra2) > M_PI)
		ra2 += 2 * M_PI;
	float distM, Rv;
	ram = (ra1 + ra2) / 2;
	dem = (de1 + de2) / 2;
	Rv = atan((ra2 - ra1) * cos(dem) / (de2 - de1 + 0.00001));
	distM = fabs((de2 - de1) / cos(Rv) * rad2deg * 60);
	Utility::spheToRect(ra1, de1, pt3);
	Utility::spheToRect(ra2, de2, pt4);
	Utility::spheToRect(ram, dem, pt5);
	(prj->*proj_func)(pt3, pt1);
	(prj->*proj_func)(pt4, pt2);
	(prj->*proj_func)(pt5, pt0);
	double angle;
	const double dx = pt1[0] - pt2[0];
	const double dy = pt1[1] - pt2[1];
	const double dq = dx * dx + dy * dy;
	const double d = sqrt(dq);
	angle = acos((pt1[1] - pt2[1]) / (d + 0.000001));
	if (pt1[0] < pt2[0])
		angle *= -1;
	clear();
	for (int j = 0; (pi_div_2 - fabs(de1 * (10 - j) / 10 + de2 * j / 10)) > 0.001; j++) {
		Utility::spheToRect((ra1 * (10 - j) / 10 + ra2 * j / 10), (de1 * (10 - j) / 10 + de2 * j / 10), pt1);
		*(dataSky++) = pt1;
		++dataSkySize;

		Utility::spheToRect((ra1 * (9 - j) / 10 + ra2 * (j + 1) / 10), (de1 * (9 - j) / 10 + de2 * (j + 1) / 10), pt2);
		*(dataSky++) = pt2;
		++dataSkySize;
	}

	if (m_dataSize != dataSkySize) {
		m_dataSize = dataSkySize;
		build();
	}
	Context::instance->transfer->endPlanCopy(vertex->get(), m_dataSize * 3 * sizeof(float));

	SkyDisplay::draw(prj,nav);

	//draw_text(prj, nav);
	std::ostringstream oss;
	Mat4f MVP = prj->getMatProjectionOrtho2D();
	Mat4f TRANSFO = Mat4f::translation(Vec3f(pt0[0], pt0[1], 0));
	TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), 3 * pi_div_2 - angle);
	oss << truncf(distM) << " nmi"; // for km *1.85185
	skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
	oss.clear();
}

SkyOrthodromy::SkyOrthodromy() : SkyDisplay(PROJECTION_TYPE::AL)
{}

void SkyOrthodromy::draw(const Projector *prj, const Navigator *nav, Vec3d equPos, Vec3d oldEquPos)
{
	if (fader.isZero()) {
		return;
	}

	Vec4f colorT (color[0], color[1], color[2], fader);
	double tempDE, tempRA, azt, altt, alt1, alt2, az1, az2;
	// for Selected position
	// calculate alt az position
	Vec3d localPos = nav->earthEquToLocal(equPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	alt1 = tempDE;
	az1 = M_PI - tempRA;
	// end of calculate alt az position

	// for Old position
	// calculate alt az position
	localPos = nav->earthEquToLocal(oldEquPos);
	Utility::rectToSphe(&tempRA, &tempDE, localPos);
	tempRA = 3 * M_PI - tempRA; // N is zero, E is 90 degrees
	if (tempRA > M_PI * 2)
		tempRA -= M_PI * 2;
	alt2 = tempDE;
	az2 = M_PI - tempRA;
	// end of calculate alt az position
	if ((az2 - az1) > M_PI)
		az1 += 2 * M_PI;
	if ((az1 - az2) > M_PI)
		az2 += 2 * M_PI;

	// Draw orthodromy
	clear();
	Utility::spheToRect(az1, alt1, pt1);
	int npoints = 21;
	float delta = (az1 - az2) / (npoints - 1);
	for (int i = 0; i < npoints; i++) {
		*(dataSky++) = pt1;
		++dataSkySize;

		azt = az1 - delta * i;
		if (az1-az2 != 0)
		  altt = atan(((tan(alt2) * sin(azt - az1)) / sin(az2 - az1)) + (tan(alt1) * sin(az2 - azt)) / sin(az2 - az1));
		else
		  altt = M_PI/2.;
		Utility::spheToRect(azt, altt, pt1);
		if (i == 12)
			pt5 = pt1;
		*(dataSky++) = pt1;
		++dataSkySize;
	}

	if (m_dataSize != dataSkySize) {
		m_dataSize = dataSkySize;
		build();
	}
	Context::instance->transfer->endPlanCopy(vertex->get(), m_dataSize * 3 * sizeof(float));

	SkyDisplay::draw(prj,nav);

	// Text
	float ang;
	ang = acos(sin(alt1) * sin(alt2) + cos(alt1) * cos(alt2) * cos(az2 - az1)) * rad2deg;
	Utility::spheToRect(az1, alt1, pt3);
	Utility::spheToRect(az2, alt2, pt4);

	(prj->*proj_func)(pt3, pt1);
	(prj->*proj_func)(pt4, pt2);
	double angle;
	const double dx = pt1[0] - pt2[0];
	const double dy = pt1[1] - pt2[1];
	const double dq = dx * dx + dy * dy;
	const double d = sqrt(dq);
	angle = acos((pt1[1] - pt2[1]) / (d + 0.000001));
	if (pt1[0] < pt2[0])
		angle *= -1;
	std::ostringstream oss;
	Mat4f MVP = prj->getMatProjectionOrtho2D();
	//localPos = nav->earthEquToLocal(pt5);
	(prj->*proj_func)(pt5, pt0);
	Mat4f TRANSFO = Mat4f::translation(Vec3f(pt0[0], pt0[1], 0));
	TRANSFO = TRANSFO * Mat4f::rotation(Vec3f(0, 0, -1), pi_div_2 - angle);
	oss << truncf(ang * 60) << " nmi"; // for km *1.85185
	skydisplay_font->print(2, -2, oss.str(), colorT, MVP * TRANSFO, 1);
	oss.clear();

}
