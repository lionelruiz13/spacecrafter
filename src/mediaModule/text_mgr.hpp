/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2014-2017 of the LSS Team & Association Sirius
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

#ifndef _TEXT_MGR_H
#define _TEXT_MGR_H

#include "tools/s_font.hpp"
#include "tools/fader.hpp"
#include "text.hpp"
#include "coreModule/projector.hpp"
#include <vector>
#include "tools/stateGL.hpp"
#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"

/**
 * @class TextMgr
 * @brief Cette classe s'ocupe de gérer toutes les entités de text utilisées 
 * à partir de commandes scripts. La classe est opérationnelle lorsque setFont
 * réussit à charger une fonte dans 7 gammes de taille.
 * 
 * isUsable est une sentinelle qui met la classe en veille lorsqu'elle n'arrive
 * pas à s'initialiser correctement.
 * 
 * Le conteneur textUsr contient tous les text qui sont indépendants
 *
 */
class TextMgr: public NoCopy {
public:
	TextMgr();
	~TextMgr();

	//! transmet les variations de temps aux différents textes
	void update(int delta_time);

	//! ordonne le tracé des différents textes
	void draw(const Projector* prj);

	//! ajoute un texte dans le conteneur textUsr
	bool add(const std::string &name, const std::string &text, int altitude, int azimuth, const std::string &size, const Vec3f &color);

	//! ajoute un texte dans le conteneur textUsr
	bool add(const std::string &name, const std::string &text, int altitude, int azimuth, const std::string &size);

	//! retire un texte du conteneur textUsr
	bool del(const std::string &name);

	//! retire tous les textes du conteneur 
	void clear();

	//! permet de changer le texte d'un text du conteneur
	void nameUpdate(const std::string &name, const std::string &text);

	//! permet de masquer un texte du conteneur
	void textDisplay(const std::string &name , bool displ);

	//! permet de changer le fading d'un texte du conteneur
	void setFadingDuration(int t);

	//! initialise l'ensemble des fontes utilisées par la classe
	void setFont(float font_size, const std::string& font_name);

	//! modifie la couleur par défaut des futurs nouveaux text 
	void setColor(const Vec3f& c);

private:
	std::vector<Text *> textUsr; // le conteneur de tous les textes
	s_font *textFont[7];		// l'ensemble de fontes utilisés 
	Vec3f defaultTextColor;		// vecteur couleur par défaut
	bool isUsable = false;		// indicateur si la classe est opérationelle
};

#endif
