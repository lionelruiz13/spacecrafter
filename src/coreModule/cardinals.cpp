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

#include "coreModule/cardinals.hpp"
#include "tools/s_texture.hpp"
#include "tools/utility.hpp"
#include <string>


#include "tools/s_font.hpp"
#include "coreModule/projector.hpp"
#include "tools/translator.hpp"



Cardinals::Cardinals(float _radius) : radius(_radius)
{
	color.set(0.6,0.2,0.2);
	// Default labels - if sky locale specified, loaded later
	// Improvement for gettext translation
}

Cardinals::~Cardinals()
{
	// if (font) delete font;
	// font = nullptr;
}

//! Draw the cardinals points : N S E W
//! handles special cases at poles
void Cardinals::draw(const Projector* prj, double latitude, bool gravityON) const
{

	if (!fader.getInterstate()) return;

	// direction text
	std::string d[4];

	d[0] = sNorth;
	d[1] = sSouth;
	d[2] = sEast;
	d[3] = sWest;

	// fun polar special cases
	if (latitude ==  90.0 ) d[0] = d[1] = d[2] = d[3] = sSouth;
	if (latitude == -90.0 ) d[0] = d[1] = d[2] = d[3] = sNorth;


	Vec4f Color(color[0],color[1],color[2],fader.getInterstate());
	Vec3f pos;
	Vec3d xy;


	float shift = font->getStrLen(sNorth)/2;

	// N for North
	pos.set(-1.f, 0.f, 0.12f);
	if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[0], Color, -shift, -shift);

	// S for South
	pos.set(1.f, 0.f, 0.12f);
	if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[1], Color, -shift, -shift);

	// E for East
	pos.set(0.f, 1.f, 0.12f);
	if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[2], Color, -shift, -shift);

	// W for West
	pos.set(0.f, -1.f, 0.12f);
	if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[3], Color, -shift, -shift);
	if ((internalNav) || (internalAstronomical)) {
		// NW
		pos.set(-0.715f, -0.685f, 0.13f);
		if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[3], Color, -shift, -shift);
		pos.set(-0.685f, -0.715f, 0.13f);
		if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[0], Color, -shift, -shift);

		// SE
		pos.set(0.715f, 0.685f, 0.13f);
		if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[2], Color, -shift, -shift);
		pos.set(0.685f, 0.715f, 0.13f);
		if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[1], Color, -shift, -shift);

		// NE
		pos.set(-0.685f, 0.715f, 0.13f);
		if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[2], Color, -shift, -shift);
		pos.set(-0.715f, 0.685f, 0.13f);
		if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[0], Color, -shift, -shift);

		// SW
		pos.set(0.715f, -0.685f, 0.13f);
		if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[1], Color, -shift, -shift);
		pos.set(0.685f, -0.715f, 0.13f);
		if (prj->projectLocal(pos,xy)) prj->printGravity180(font, xy[0], xy[1], d[3], Color, -shift, -shift);
	}
}


//! Translate cardinal labels with gettext to current sky language
void Cardinals::translateLabels(Translator& trans)
{
	sNorth = trans.translateUTF8("N");
	sSouth = trans.translateUTF8("S");
	sEast = trans.translateUTF8("E");
	sWest = trans.translateUTF8("W");

	if(font) font->clearCache();
}
