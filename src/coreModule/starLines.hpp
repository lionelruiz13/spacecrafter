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
#include <memory>
#include "tools/vecmath.hpp"

#include "tools/fader.hpp"
//

#include "tools/no_copy.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"

using HIPpos = std::pair<int, Vec3f>;

class Navigator;
class Projector;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;

/*! \class StarLines
  * \brief classe représentant un astérisme customisé par l'utilisateur
  *
  *  \details Cette classe dessine un astérisme customisé à partir d'un catalogue
  *  d'étoiles HIP chargé en amont.
  */
class StarLines: public NoCopy  {
public:
	StarLines();
	~StarLines();

	//! update les faders de la classe
	void update(int delta_time) {
		showFader.update(delta_time);
	}

	//! fixe l'état du fader
	void setFlagShow(bool b) {
		showFader = b;
	}

	void setFlagSelected(bool b) {
		selectedFader = b;
	}

	//! récupère l'état du fader
	bool getFlagSelected(void) const {
		return selectedFader;
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

	//! \brief charge une étoile pour le catalogue
	void loadHipStar(int name, Vec3f position ) noexcept;

	//! \brief charge un fichier utilisateur d'astérismes dans les tampons
	//! \param fileName représente le nom complet du fichier des datas
	bool loadData(const std::string& fileName) noexcept;

	//! \brief charge un seul asterisme dans les tampons
	//! \param fileName représente le nom complet du fichier des datas
	void loadStringData(const std::string& record) noexcept;

	//! \brief lit le catalogue des étoiles les plus lumineuses
	//! \return true si tout est oki, false sinon
	bool loadCat(const std::string& fileName, bool useBinary) noexcept;

	//! \brief sauvegarde le catalogue des étoiles les plus lumineuses
	//! \return true si tout est oki, false sinon
	bool saveCat(const std::string& fileName, bool useBinary) noexcept;

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
	//! \brief lit le catalogue des étoiles les plus lumineuses
	//! \return true si tout est oki, false sinon
	bool loadHipCat(const std::string& fileName) noexcept;
	//! \brief lit le catalogue binaire des étoiles les plus lumineuses
	//! \return true si tout est oki, false sinon
	bool loadHipBinCat(const std::string& fileName) noexcept;

	//! sauvegarde le catalogue des étoiles les plus lumineuses
	//! \return true si tout est oki, false sinon
	bool saveHipCat(const std::string& fileName) noexcept;
	//! sauvegarde le catalogue binaire des étoiles les plus lumineuses
	//! \return true si tout est oki, false sinon
	bool saveHipBinCat(const std::string& fileName) noexcept;

	// shader d'affichage
	//std::unique_ptr<shaderProgram> shaderStarLines;
	// context
	VkCommandBuffer cmds[3];
	bool needRebuild[3] {true, true, true};
	// données VAO-VBO
	std::unique_ptr<VertexArray> m_dataGL;
	std::unique_ptr<VertexBuffer> vertex;
	// uniform pattern of pipeline
	std::unique_ptr<PipelineLayout> layout;
	// whole draw context
	std::unique_ptr<Pipeline> pipeline;
	// set of local uniforms
	std::unique_ptr<Set> set;
	struct frag {
		Vec3f color;
		float fader;
	};
	std::unique_ptr<SharedBuffer<Mat4f>> uMat;
	std::unique_ptr<SharedBuffer<frag>> uFrag;
	// initialise le shader
	void createSC_context();
	//! build VertexArray and fill it with new datas
	void rebuild(std::vector<float> &vertexData);
	//! build or rebuild command with new VertexArray and draw
	void rebuildCommand(int idx);
	// supprime le shader
	// void deleteShader();
	// tampon d'affichage
	std::vector<float> linePos;
	// conteneur des étoiles servant à créer les sommets des lignes
	std::vector<HIPpos> HIP_data;
	// recherche dans HIP_data les coordonnées d'une étoile
	Vec3f searchInHip(int HIP);
	// fader d'affichage
	LinearFader showFader;
	LinearFader selectedFader;
	// indique si l'on doit recharger les tampons dans la CG
	//bool isModified = true;
	// couleur du tracé des lines
	Vec3f lineColor;
	// fonction de tracé effectif des asterismes suviant le mode choisi
	void drawGL(Mat4f& matrix) noexcept;
};
#endif
