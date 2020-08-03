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


#include <map>
#include <memory>

#include "tools/s_font.hpp"
#include "tools/fader.hpp"
#include "text.hpp"
#include "coreModule/projector.hpp"
#include "tools/stateGL.hpp"
#include "tools/vecmath.hpp"
#include "tools/no_copy.hpp"

/**
 * \file text_mgr.hpp
 * \brief This class processes all text entities used by the user. 
 * \author Olivier NIVOIX
 * \version 2
 *
 * \class TextMgr
 * 
 * \brief This class processes all text entities used by the user.
 * 
 * The class is operational when setFont succeeds in loading a font in 7 size ranges.
 * A sentry who puts the class on standby when it cannot initialize properly.
 * 
 * It provides classic access to the management of horizontal texts.
 * 
 * The container contains all the texts which are independent of each other.
 * The name of the text serves as a key. (uniqueness)
 *
 */

struct TEXT_MGR_PARAM {
    std::string string;
    float altitude;
    float azimuth;
    std::string fontSize;
    std::string textAlign;
    Vec3f color;
    bool useColor;
};

class TextMgr: public NoCopy {
public:
	TextMgr();
	~TextMgr();
	//! transmet les variations de temps aux différents textes
	void update(int delta_time);

	//! ordonne le tracé des différents textes
	void draw(const Projector* prj);

	//! ajoute un texte dans le conteneur textUsr
	void add(const std::string& name, const TEXT_MGR_PARAM& textParam);

	//! retire un texte du conteneur textUsr
	void del(const std::string &name);

	//! retire tous les textes du conteneur 
	void clear();

	//! permet de changer le texte d'un text du conteneur
	void textUpdate(const std::string &name, const std::string &text);

	//! permet de masquer un texte du conteneur
	void textDisplay(const std::string &name , bool displ);

	//! permet de changer le fading d'un texte du conteneur
	void setFadingDuration(float t) {
		fadingDuration = t;
	}

	//! initialise l'ensemble des fontes utilisées par la classe
	void setFont(float font_size, const std::string& font_name);

	//! modifie la couleur par défaut des futurs nouveaux text 
	void setColor(const Vec3f& c);

private:
	void clearCache();			// vide les caches des fonts
	std::map<std::string, std::unique_ptr<Text>> textUsr; // le conteneur de tous les textes
	std::map<std::string, FONT_SIZE> strToFontSize; // convertir txt to FONT_SIZE
	std::map<std::string, TEXT_ALIGN> strToTextAlign; // convertir txt to TEXT_POSITION
	std::vector<std::shared_ptr<s_font>> textFont;		// l'ensemble de fontes utilisés
	Vec3f defaultTextColor;		// vecteur couleur par défaut
	bool isUsable = false;		// indicateur si la classe est opérationelle
	float fadingDuration;		// durée d'une fading de text (s'il existe) en secondes
	std::string m_fontName;		// remembers what fontName is used
	float m_fontSize; 			// remembers what fontSize is used
};

#endif


