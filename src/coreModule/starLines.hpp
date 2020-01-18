/*
* This source is the property of Immersive Adventure
* http://immersiveadventure.net/
*
* It has been developped by part of the LSS Team.
* For further informations, contact:
*
* albertpla@immersiveadventure.net
*
* This source code mustn't be copied or redistributed
* without the authorization of Immersive Adventure
* (c) 2017 - all rights reserved
*
*/

#ifndef STARLINES_HPP
#define STARLINES_HPP

#include <vector>
#include "tools/vecmath.hpp"
#include <GL/glew.h>
#include "tools/fader.hpp"
#include "tools/shader.hpp"
#include "tools/stateGL.hpp"


using HIPpos = std::pair<int, Vec3f>;

class Navigator;
class Projector;

/*! \class StarLines
  * \brief classe représentant un astérisme customisé par l'utilisateur
  *
  *  \details Cette classe dessine un astérisme customisé à partir d'un catalogue
  *  d'étoiles HIP chargé en amont.
  */
class StarLines {
public:
	StarLines();
	~StarLines();
	StarLines(StarLines const &) = delete;
	StarLines& operator = (StarLines const &) = delete;

	//! update les faders de la classe
	void update(int delta_time) {
		showFader.update(delta_time);
	}

	//! fixe l'état du fader
	void setFlagShow(bool b) {
		showFader = b;
	}

	//! récupère l'état du fader
	bool getFlagShow(void) const {
		return showFader;
	}

	//! indique la couleur du tracé
	void setColor(const Vec3f& c) {
		lineColor = c;
	}

	//! renvoie la couleur du tracé
	const Vec3f& getColor() {
		return lineColor;
	}

	//! \brief charge un fichier utilisateur d'astérismes dans les tampons
	//! \param fileName représente le nom complet du fichier des datas
	bool loadData(std::string fileName) noexcept;

	//! \brief charge un seul asterisme dans les tampons
	//! \param fileName représente le nom complet du fichier des datas
	void loadStringData(std::string record) noexcept;

	//! \brief lit le catalogue des étoiles les plus lumineuses
	//! \return true si tout est oki, false sinon
	bool loadHipCatalogue(std::string fileName) noexcept;

	//! \brief lit le catalogue binaire des étoiles les plus lumineuses
	//! \return true si tout est oki, false sinon
	bool loadHipBinCatalogue(std::string fileName) noexcept;

	//! dessine les asterismes issus de loadData IN_SOLARSYSTEM
	void draw(const Projector* prj) noexcept;
	//! dessine les asterismes issus de loadData dans IN_GALAXY
	void draw(const Navigator * nav) noexcept;

	//! Supprime le contenu des tampons d'affichage
	void drop();

	//! Supprime le contenu du catalogue d'étoiles
	void clear() {
		HIP_data.clear();
	}

protected:
	// shader d'affichage
	shaderProgram *shaderStarLines;
	// données VAO-VBO
	DataGL starLines;
	// initialise le shader
	void createShader();
	// supprime le shader
	void deleteShader();
	// tampon d'affichage
	std::vector<float> linePos;
	// conteneur des étoiles servant à créer les sommets des lignes
	std::vector<HIPpos> HIP_data;
	// recherche dans HIP_data les coordonnées d'une étoile
	Vec3f searchInHip(int HIP);
	// fader d'affichage
	LinearFader showFader;
	// état global de la classe
	bool isAlive;
	// indique si l'on doit recharger les tampons dans la CG
	bool isModified = true;
	// couleur du tracé des lines
	Vec3f lineColor;
	// fonction de tracé effectif des asterismes suviant le mode choisi
	void draw(Mat4f& matrix) noexcept;
};
#endif
