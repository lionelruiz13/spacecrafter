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

#ifndef STARNAVIGATOR_HPP
#define STARNAVIGATOR_HPP

#include <vector>
#include <mutex>

#include <memory>

#include "tools/vecmath.hpp"
//#include "inGalaxyModule/starManager.hpp"
#include "tools/ThreadPool.hpp"
#include "tools/no_copy.hpp"
#include "tools/object_type.hpp"
#include "tools/object.hpp"
#include "starModule/geodesic_grid.hpp"
#include "EntityCore/Resource/SharedBuffer.hpp"
#include "tools/fader.hpp"
#include "tools/ScModule.hpp"


/*! \class StarNavigator
  * \brief class allowing the stroll in the stars
  *
  *  \details The class is responsible for processing all the stars to
  * allow a trip in space (basic unit: the parsec)
  */

class Projector;
class Navigator;
class ToneReproductor;
class VertexArray;
class VertexBuffer;
class Pipeline;
class PipelineLayout;
class Set;
class StarViewer;
class s_texture;
struct starInfo;
class StarManager;

typedef std::tuple<double, double, const std::string , const Vec4f > starDBtoDraw;

class StarNavigator: public NoCopy , public ModuleFont, public ModuleFader<LinearFader> {
public:
	StarNavigator();
	~StarNavigator();

	/*! /fn
	 * \brief Loads the star catalog into memory
	 * \param fileName, the full name of the data file
	 * \param mode, what type of data the program should read
	 * \param binaryData: read binary data or not.
	 */
	void loadRawData(const std::string &fileName) noexcept;
	void loadOtherData(const std::string &fileName) noexcept;
	void loadData(const std::string &fileName, bool binaryData) noexcept;

	void saveData(const std::string &fileName, bool binaryData) noexcept;
	/*! /fn
	 * \brief displays the stars of the catalog on the screen
	 * \param nav for marker change matrices
	 * \param prj (not used)
	 */
	void draw(const Navigator * nav, const Projector* prj) noexcept;
	void drawRaw(const Mat4f &matrix) const noexcept;
	/*! /fn
	 * \brief calculates the stars to display from the structure
	 * \param posI, position in parsec of the observer with respect to the sun
	 * \todo what to do in case of error ?
	 */
	void computePosition(Vec3f posI) noexcept;
	//! Build draw command
	void build();

	//! Update any time-dependent features.
	//! Includes fading in and out stars and labels when they are turned on and off.
	virtual void update(double deltaTime) {
		names_fader.update(deltaTime);
		fader.update(deltaTime);
	}

	void setMagConverterMagShift(float s){
		mag_shift = s;
		needComputeRCMagTable = true;
	}

	void setMagConverterMaxMag(float mag) {
		max_mag = mag;
		needComputeRCMagTable = true;
	}

	//! Set maximum magnitude at which stars names are displayed.
	void setMaxMagName(float b) {
		maxMagStarName=b;
	}
	//! Get maximum magnitude at which stars names are displayed.
	float getMaxMagName(void) const {
		return maxMagStarName;
	}

	void setStarSizeLimit(float f) {
		starSizeLimit = f;
		needComputeRCMagTable = true;
	}

	void setScale(float s) {
		starScale = s;
		needComputeRCMagTable = true;
	}

	void setMagScale(float s) {
		starMagScale = s;
		needComputeRCMagTable = true;
	}

	//! Set display flag for Stars.
	void setFlagStars(bool b) {
		starsFader=b;
		needComputeRCMagTable = true;
	}

	bool getFlagStars() {
		return starsFader;
	}

	//! Sets the time it takes for star names to fade and off.
	//! @param duration the time in seconds.
	void setNamesFadeDuration(float duration) {
		names_fader.setDuration((int) (duration * 1000.f));
	}

	//! Set display flag for Star names (labels).
	void setFlagNames(bool b) {
		names_fader=b;
	}

	//! Get display flag for Star names (labels).
	bool getFlagNames(void) const {
		return names_fader==true;
	}

	void clear(){
		listGlobalStarVisible.clear();
		needComputeRCMagTable = true;
	}

	starInfo* getStarInfo(unsigned int HIPName) const;

	std::vector<ObjectBaseP> searchAround(Vec3d v, double limitFov, const Navigator *nav);

private:
	//buffers for displaying shaders
	float *starVec;


	void drawStarName(const Projector* prj);

	LinearFader names_fader;
	bool starsFader = true;
	std::vector<starDBtoDraw> starNameToDraw;

	// observer's position in parsec
	Vec3f pos;
	// pos value at the front frame
	Vec3f old_pos;

	//the manager of the set of stars
	std::unique_ptr<StarManager> starMgr;
	// shader used for display
	//std::unique_ptr<shaderProgram> shaderStarNav;
	// VAO-VBO management structure
	VkCommandBuffer cmds[3];
	std::unique_ptr<VertexArray> m_dataGL;
	std::unique_ptr<VertexBuffer> vertex;
	std::unique_ptr<PipelineLayout> layout;
	std::unique_ptr<Pipeline> pipeline;
	std::unique_ptr<Set> set;
	std::unique_ptr<SharedBuffer<Mat4f>> uMat;
	std::unique_ptr<StarViewer> starViewer;
	std::unique_ptr<SharedBuffer<VkDrawIndirectCommand>> drawData;
	//initialization of shader and VAO-VBO
	void createSC_context();

	//precalculation of the color table
	void computeRCMagTable();
	//list of stars to display from the StarManager
	std::vector<starInfo*> listGlobalStarVisible;
	// size of the listGlobalStarVisible
	unsigned int maxStars;
	int nbStars;
	//function to set listGlobalStarVisible
	void setListGlobalStarVisible();
	//function to set buffers for shaders to zero
	void clearBuffer();

	// function to calculate the colors and radius of a star according to its magnitude
	int computeRCMag(float mag, const ToneReproductor *eye, float rc_mag[2]);

	//color table
	static Vec3f color_table[128];

	//texture used to display a star
	s_texture *starTexture;
	// table showing the radius and intensity of colors
	float rc_mag_table[2*256];

	//mutex on display buffers
	std::mutex accessTab;
	// sub-function for computePosition threads
	bool computeChunk(unsigned int first, unsigned int last);

	float mag_shift = 0.f;	//<stars, mag_converter_mag_schift>
	float max_mag= 6.5f;		//<stars, mag_converter_max_mag>
	float maxMagStarName;
	float starScale = 0.9f; // <stars, star_scale>
	float starMagScale = 0.9f; // <stars,star_mag_scale>
	float starSizeLimit = 9.0; // <astro:star_size_limit>

	bool needComputeRCMagTable = true;

	static bool threadWrapper(StarNavigator *a, int first, unsigned int last)
		{return a->computeChunk(first, last);};

	ThreadPool *pool=nullptr;
	std::vector< std::future<bool> > results;
};

#endif
