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
#include "tools/ScModule.hpp"
#include "tools/vecmath.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "tools/object_base.hpp"

//! Class which manages the Tully Galaxies catalog

class Projector;
class Navigator;
class s_texture;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;
class VolumObj3D;

typedef std::tuple<double, double, const std::string , const Vec4f > starDBtoDraw;

class Tully: public ModuleFont {
public:
	Tully();
	~Tully();

	//! displays the point cloud
	void draw(double distance, const Navigator *nav, const Projector *prj) noexcept;

	//! update fader
	void update(int delta_time) {
		names_fader.update(delta_time);
		fader.update(delta_time);
	}

	//! changes the fader duration
	void setFaderDuration(float duration) {
		fader.setDuration((int)(duration*1000.f));
	}

	//! Sets the time it takes for star names to fade and off.
	//! @param duration the time in seconds.
	void setNamesFadeDuration(float duration) {
		names_fader.setDuration((int) (duration * 1000.f));
	}

	//! modify the fader
	void setFlagShow(bool b) {
		fader = b;
	}

	//! returns the value of the fader
	bool getFlagShow(void) const {
		return fader;
	}

	//! Set display flag for Star names (labels).
	void setFlagNames(bool b) {
		names_fader=b;
	}

	//! Get display flag for Star names (labels).
	bool getFlagNames(void) const {
		return names_fader==true;
	}

	void setWhiteColor(bool b) {
		useWhiteColor = b;
	}

	bool getWhiteColor() {
		return useWhiteColor;
	}

	//! allows to update the texture of the galaxies
	void setTexture(const std::string& tex_file/*, const std::string& tex_file_small*/);

	//! read the data from the passed catalog whose name is passed in parameter
	bool loadCatalog(const std::string &cat) noexcept;

	//! read the catalog to display with a fading of min(distance/optimalDistance, 1)
	bool loadBigCatalog(const std::string &cat, float optimalDistance) noexcept;

	//! Return true if tully need to be build before being drawn
	bool mustBuild() const {
		return needRebuild;
	}

	// Build the draw commands for the loaded catalogs and texture, doesn't modify assigned withObject
	void buildInternal();

	// Build the draw commands for the loaded catalogs and texture
	void build(VolumObj3D *withObject = nullptr) {
		if (!isAlive)
			return;
		this->withObject = withObject;
		includeObject = (withObject != nullptr);
		buildVertexSplit();
		buildInternal();
	}

	std::vector<ObjectBaseP> searchAround(Vec3d v, double limitFov, const Navigator *nav);

private:
	// initialize ShaderPoints and ShaderSquare shaders and vao-vbo
	void createSC_context();

	void computeSquareGalaxies(Vec3f camPosition);

	//! Initialize the vertex buffer splitting for the object
	void buildVertexSplit();

	void drawGalaxyName(const Projector* prj);

	s_texture* texGalaxy;
	LinearFader fader;
	LinearFader names_fader;
	std::vector<starDBtoDraw> galaxyNameToDraw;

	//camera position
	Vec3f camPos;
	//fixed float array for openGL buffers
	std::vector<std::string> nameTully;
	std::vector<float> posTully;
	std::vector<float> colorTully;
	std::vector<float> texTully;
	std::vector<float> scaleTully;
	// Hold the data as they are stored in vertexPoints
	std::vector<float> sortedDataTully;

	struct tmpTully {
		Vec3f position;
		Vec3f color;
		float distance;
		float radius;
		float texture;
		uint8_t planeSide;
		std::string name;
	};
	static bool compTmpTully(const tmpTully &a,const tmpTully &b);

	std::list<tmpTully> lTmpTully;

	//return the number of galaxies read from the catalog(s)
	unsigned int nbGalaxy;
	bool isAlive = false;
	bool needRebuild = false;
	bool useWhiteColor = true;
	// returns the number of different textures in the texture
	int nbTextures;
	// Vulkan data
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<Set> set, bigSet;
	struct s_geom {
		Mat4f mat;
		Vec3f camPos;
		int nbTextures;
	};
	std::unique_ptr<SharedBuffer<s_geom>> uGeom;
	std::unique_ptr<SharedBuffer<float>> uFader;
	std::unique_ptr<SharedBuffer<float>> uBigFader;
	float bigCatalogMaxVisibilityAt = 1;
	// drawData for square back, square front, point back, point front
	std::unique_ptr<SharedBuffer<VkDrawIndirectCommand[4]>> drawData;
	VkCommandBuffer cmdCustomColor[6];
	VkCommandBuffer *cmdWhiteColor = nullptr;
	Pipeline *pipelinePoints;
	Pipeline *pipelineSquare;
	std::unique_ptr<VertexArray> m_pointsGL;
	std::unique_ptr<VertexArray> m_squareGL;
	std::unique_ptr<VertexBuffer> vertexPoints;
	std::unique_ptr<VertexBuffer> vertexSquare;
	std::unique_ptr<VertexBuffer> vertexPointsExt; // Big catalog points
	VolumObj3D *withObject = nullptr;
	uint32_t drawDataPointFirstOffset;
	uint32_t drawDataPointSecondSize;
	uint8_t planeOrder = 0; // 0 = plane 0 behind plane 1, 1 = plane 1 behind plane 0
	bool includeObject = false;
};

#endif // ___TULLY_HPP___
