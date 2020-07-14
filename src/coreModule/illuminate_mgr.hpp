/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2015 of the LSS Team & Association Sirius
 * Copyright (C) 2020 of the LSS Team & Association Sirius
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

#ifndef _ILLUMINATE_MGR_H_
#define _ILLUMINATE_MGR_H_

#include <vector>
#include <memory>

#include "tools/object.hpp"
#include "tools/fader.hpp"
#include "coreModule/grid.hpp"
#include "coreModule/illuminate.hpp"
#include "tools/no_copy.hpp"
#include "tools/SphereGrid.hpp"

class HipStarMgr;
class Navigator;
class ConstellationMgr;
class VertexArray;
class shaderProgram;

/*! \class IlluminateMgr
  * \brief handles all illuminate stars from Hipparcos catalog for better stars visualisation.
  * \author Olivier NIVOIX
  * \date 13 mai 2020
  *
  * @section DESCRIPTION
  * Cette classe stocke dans la structure illumianteGrid un illuminate, objet visant à recouvrir une étoile d'une texture
  * afin de la rendre plus visible sous le dôme.
  *
  * Les étoiles sont issues du catalogue Hipparcos.
  *
  */
class IlluminateMgr: public NoCopy {
public:
	IlluminateMgr(HipStarMgr *_hip_stars, Navigator *_navigator, ConstellationMgr *_asterism);
	virtual ~IlluminateMgr();

	// indique la taille d'affichage des illuminates par défaut
	void setDefaultSize(double v) {
		defaultSize =v;
	}

	// construit l'illuminate d'une étoile
	void load(int num, double size, double rotation);
	// construit l'illuminate d'une étoile en indiquant la couleur utilisée
	void load(int num, const Vec3f& _color, double _size, double rotation);

	//! remove user added Illuminate
	void remove(unsigned int name);

	//! remove all user added Illuminate
	void removeAll();

	//! Draw all the Illuminate
	void draw(Projector *prj, const Navigator *nav);

	//! change la texture par defaut des illuminates par le fichier proposé en paramètre
	void changeTex(const std::string& fileName);
	//!	supprime la texture définie par l'utilisateur
	void removeTex();

	//! charge toutes les étoiles de l'asterism d'une constellation
	void loadConstellation(const std::string& abbreviation, double size, double rotation);
	//! charge toutes les étoiles de l'asterism d'une constellation en imposant la couleur du groupement
	void loadConstellation(const std::string& abbreviation, const Vec3f& color, double size, double rotation);
	//! charge toutes les étoiles de tous les asterismes d'une sky_culture
	void loadAllConstellation(double size, double rotation);

	//! supprime toutes les étoiles de l'asterism d'une constellation
	void removeConstellation(const std::string& abbreviation);
	//! supprime toutes les étoiles de tous les asterismes d'une sky_culture
	void removeAllConstellation();

private:
	//! Load an individual Illuminate with all data
	void loadIlluminate(unsigned int name, double ra, double de, double angular_size, double r, double g, double b, double tex_rotation);

	/// std::vector<Illuminate*> illuminateArray; 		//!< The Illuminate vector
	/// std::vector<Illuminate*>* illuminateZones;		//!< array of Illuminate vector with the grid id as array rank
	/// LittleGrid illuminateGrid;					//!< Grid for display opimisation
	SphereGrid<Illuminate*> illuminateGrid;

	double defaultSize;							//!< defautl Size from illuninate if not precised


	HipStarMgr* hip_stars = nullptr;			//!< provide acces point to HipStarMgr
	Navigator* navigator = nullptr;				//!< provide acces point to Navigator
	ConstellationMgr* asterism= nullptr;		//!< provide acces point to ConstellationMgr

	std::unique_ptr<shaderProgram> m_shaderIllum;	//!< shader how draw all illuminate
	std::unique_ptr<VertexArray> m_illumGL;
	std::vector<float> illumPos;
	std::vector<float> illumTex;
	std::vector<float> illumColor;

	s_texture * currentTex = nullptr;			//!< Pointer of texture used to draw
	s_texture * defaultTex = nullptr;		//!< Common texture if no other texture defined
	s_texture * userTex = nullptr;				//!< Texture define by user

	void createSC_context();
};

#endif // _ILLUMINATE_MGR_H_
