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
  * \brief class representing an asterism customized by the user
  *
  *  \details This class draws a custom asterism from an uploaded HIP star catalog.
  */
class StarLines: public NoCopy  {
public:
	StarLines();
	~StarLines();

	//! update the faders of the class
	void update(int delta_time) {
		showFader.update(delta_time);
	}

	//! set the state of the fader
	void setFlagShow(bool b) {
		showFader = b;
	}

	void setFlagSelected(bool b) {
		selectedFader = b;
	}

	//! retrieve the state of the fader
	bool getFlagSelected(void) const {
		return selectedFader;
	}

	//! retrieves the state of the fader
	bool getFlagShow(void) const {
		return showFader;
	}

	//! indicates the color of the trace
	void setColor(const Vec3f& c) {
		lineColor = c;
	}

	//! rsends the color of the track
	const Vec3f& getColor() {
		return lineColor;
	}

	//! \brief loads a star for the catalog
	void loadHipStar(int name, Vec3f position ) noexcept;

	//! \brief loads a user file of asterisks in the buffers
	//! \param fileName represents the full name of the data file
	bool loadData(const std::string& fileName) noexcept;

	//! \brief loads a single asterism in the buffers
	//! \param fileName represents the full name of the dataset
	void loadStringData(const std::string& record) noexcept;

	//! \brief reads the catalog of the brightest stars
	//! \return true if everything is oki, false otherwise
	bool loadCat(const std::string& fileName, bool useBinary) noexcept;

	//! \brief save the catalog of the brightest stars
	//! \return true if all is oki, false otherwise
	bool saveCat(const std::string& fileName, bool useBinary) noexcept;

	//! draw asterisms from loadData IN_SOLARSYSTEM
	void draw(const Projector* prj) noexcept;
	//! draw asterisms from loadData in IN_GALAXY
	void draw(const Navigator * nav) noexcept;

	//! Delete the content of the display buffers
	void drop();

	//! Delete the content of the star catalog
	void clear() {
		HIP_data.clear();
	}

protected:
	//! \brief reads the catalog of the brightest stars
	//! \return true if all is oki, false otherwise
	bool loadHipCat(const std::string& fileName) noexcept;
	//! \brief reads the binary catalog of the brightest stars
	//! \return true if everything is oki, false otherwise
	bool loadHipBinCat(const std::string& fileName) noexcept;

	//! sausaves the catalog of the brightest stars
	//! \return true if all is oki, false otherwise
	bool saveHipCat(const std::string& fileName) noexcept;
	//! save the binary catalog of the brightest stars
	//! \return true if everything is oki, false otherwise
	bool saveHipBinCat(const std::string& fileName) noexcept;

	// display shader
	//std::unique_ptr<shaderProgram> shaderStarLines;
	// context
	VkCommandBuffer cmds[3];
	bool needRebuild[3] {true, true, true};
	// data VAO-VBO
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
	// initialize the shader
	void createSC_context();
	//! build VertexArray and fill it with new datas
	void rebuild(std::vector<float> &vertexData);
	//! build or rebuild command with new VertexArray and draw
	void rebuildCommand(int idx);
	// deletes the shader
	// void deleteShader();
	// display buffer
	std::vector<float> linePos;
	// stars container used to create the vertices of the lines
	std::vector<HIPpos> HIP_data;
	// search in HIP_data the coordinates of a star
	Vec3f searchInHip(int HIP);
	// display fader
	LinearFader showFader;
	LinearFader selectedFader;
	// indicates if we have to reload the buffers in the CG
	//bool isModified = true;
	// color of the line drawing
	Vec3f lineColor;
	// function of effective drawing of asterisms according to the chosen mode
	void drawGL(Mat4f& matrix) noexcept;
};
#endif
