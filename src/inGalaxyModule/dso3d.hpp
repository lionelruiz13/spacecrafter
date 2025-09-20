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
* (c) 2017 - 2020 all rights reserved
*
*/

#ifndef ___DSO3D_HPP___
#define ___DSO3D_HPP___

#include <string>
#include <fstream>
#include <vector>
#include <memory>

#include "tools/fader.hpp"
#include "tools/vecmath.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "tools/ScModule.hpp"

//! Class which manages the DSO Catalog for in_galaxy

class Projector;
class Navigator;
class s_texture;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;

class Dso3d: public ModuleFont {
public:
	Dso3d();
	~Dso3d();

	//! displays the point cloud
	void draw(double distance, const Projector *prj,const Navigator *nav) noexcept;

	void drawDsoName(const Projector* prj, const Navigator *nav);

	//! update fader
	void update(int delta_time) {
		fader.update(delta_time);
		names_fader.update(delta_time);
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

	//! Set display flag for names.
	void setFlagNames(bool b) {
		names_fader=b;
	}

	//! Get display flag for names.
	bool getFlagNames(void) const {
		return names_fader==true;
	}

	//! Define the default Label Color for DSO font
	void setLabelColor(const Vec3f& c) {
		labelColor = c;
	}

	//! get the actual Label Color for DSO font
	const Vec3f &getLabelColor(void) const {
		return labelColor;
	}

	//! allows to update the texture of the nebulae
	void setTexture(const std::string& tex_file);

	//! read data from the catalog whose name is passed in parameter
	bool loadCatalog(const std::string &cat) noexcept;

	//! add Dso with command line
	bool loadCommand(int typeDso, std::string name, float size, float alpha, float delta, float distance, int xyz);
	void removeSupplementalDso();

	//! Build draw command
	void build();
private:
	// iinitialize the shader
	void createSC_context();
	// set the number of textures in texNebulae
	int nbTextures;
	// camera position
	Vec3f camPos;
	std::unique_ptr<s_texture> texNebulae;
	// fader for display
	LinearFader fader;
	LinearFader names_fader;
	//float array for openGL buffer
	std::vector<float> posDso3d;
	std::vector<float> scaleDso3d;
	std::vector<float> texDso3d;
	std::vector<std::string> nameDso3d;
	//returns the number of nebulae read from the catalog(s)
	unsigned int nbNebulae;
	std::unique_ptr<VertexArray> sData;
	std::unique_ptr<VertexBuffer> vertex;
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<Set> set;
	VkCommandBuffer cmds[3];
	struct pGeom_s {
		Mat4f Mat;
		Vec3f camPos;
		int nbTextures;
	};
	std::unique_ptr<SharedBuffer<pGeom_s>> uGeom;
	std::unique_ptr<SharedBuffer<float>> uFader;
	Vec3f labelColor;
};

#endif // ___DSO3D_HPP___
