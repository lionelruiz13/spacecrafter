/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2017-2020 of the LSS Team & Association Sirius
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

#ifndef ___TULLY_HPP___
#define ___TULLY_HPP___

#include <string>
#include <fstream>
#include <vector>
#include <list>
#include <memory>

#include "tools/fader.hpp"
#include "renderGL/stateGL.hpp"
#include "tools/vecmath.hpp"


//! Class which manages the Tully Galaxies catalog

class Projector;
class Navigator;
class s_texture;
class VertexArray;
class shaderProgram;

class Tully {
public:
	Tully();
	~Tully();

	//! affiche le nuage de points
	void draw(double distance, const Projector *prj,const Navigator *nav) noexcept;

	//! mise à jour du fader
	void update(int delta_time) {
		fader.update(delta_time);
	}

	//! modifie la durée du fader
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}

	//! modifie le fader
	void setFlagShow(bool b) {
		fader = b;
	}

	//! renvoie la valeur du fader
	bool getFlagShow(void) const {
		return fader;
	}

	void setWhiteColor(bool b) {
		useWhiteColor = b;
	}

	bool getWhiteColor() {
		return useWhiteColor;
	}

	//! permet de mettre à jour la texture des galaxies
	void setTexture(const std::string& tex_file/*, const std::string& tex_file_small*/);

	//! lecture des données du catalogue passé dont le nom est passé en paramètre 
	bool loadCatalog(const std::string &cat) noexcept;

private:
	// initialise les shaders ShaderPoints et ShaderSquare ainsi que les vao-vbo
	void createSC_context();

	void computeSquareGalaxies(Vec3f camPosition);

	s_texture* texGalaxy;
	LinearFader fader;

	//position camera
	Vec3f camPos;
	//tableau de float fixe pour tampons openGL
	std::vector<float> posTully;
	std::vector<float> scaleTully;
	std::vector<float> colorTully;
	std::vector<float> texTully;

	//tableau de float temporaire pour tampons openGL
	std::vector<float> posTmpTully;
	std::vector<float> texTmpTully;
	std::vector<float> radiusTmpTully;

	struct tmpTully {
		Vec3f position;
		float distance;
		float radius;
		float texture;
	};
	static bool compTmpTully(const tmpTully &a,const tmpTully &b);

	std::list<tmpTully> lTmpTully;

	//renvoie le nombre de galaxies lues du/des catalogues
	unsigned int nbGalaxy;
	bool isAlive = false;
	bool useWhiteColor = true;
	// renvoie le nombre des différentes textures dans la texture
	int nbTextures;
	// données openGL
	std::unique_ptr<VertexArray> m_pointsGL;
	std::unique_ptr<VertexArray> m_squareGL;
	// shader responsable de l'affichage du nuage
	std::unique_ptr<shaderProgram> shaderPoints;
	std::unique_ptr<shaderProgram> shaderSquare;
};

#endif // ___TULLY_HPP___
