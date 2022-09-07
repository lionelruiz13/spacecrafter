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

// max amount of illuminate simultaneously displayed
#define MAX_ILLUMINATE 120416

#include <vector>
#include <memory>

#include "tools/object.hpp"
#include "tools/fader.hpp"
#include "coreModule/illuminate.hpp"
#include "tools/no_copy.hpp"
#include "tools/SphereGrid.hpp"

class HipStarMgr;
class Navigator;
class ConstellationMgr;
class VertexArray;
class VertexBuffer;
class PipelineLayout;
class Pipeline;
class Set;

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
	IlluminateMgr(std::shared_ptr<HipStarMgr> _hip_stars, Navigator *_navigator, std::shared_ptr<ConstellationMgr> _asterism);
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
	void loadIlluminate(unsigned int name, double ra, double de, double angular_size, double r, double g, double b, float tex_rotation);
	void buildSet();

	#ifdef _MSC_VER // MSVC is not C++11 compliant, using copy for moving in resize and reserve
	SphereGrid<std::shared_ptr<Illuminate>> illuminateGrid;
	#else
	SphereGrid<std::unique_ptr<Illuminate>> illuminateGrid;
	#endif

	double defaultSize;							//!< defautl Size from illuninate if not precised


	std::shared_ptr<HipStarMgr> hip_stars;			//!< provide acces point to HipStarMgr
	Navigator* navigator = nullptr;				//!< provide acces point to Navigator
	std::shared_ptr<ConstellationMgr> asterism;		//!< provide acces point to ConstellationMgr

	int cmds[3];
	std::unique_ptr<PipelineLayout> m_layoutIllum;
	std::unique_ptr<Pipeline> m_pipelineIllum;
	std::unique_ptr<VertexArray> m_illumGL;
	std::unique_ptr<VertexBuffer> vertex;
	SubBuffer index;
	std::unique_ptr<Set> m_setIllum;

	std::shared_ptr<s_texture> currentTex;		//!< Pointer of texture used to draw
	std::shared_ptr<s_texture> defaultTex;		//!< Common texture if no other texture defined

	void createSC_context();
};

#endif // _ILLUMINATE_MGR_H_
