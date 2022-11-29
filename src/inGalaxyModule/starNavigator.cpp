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

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>
#include <thread>


#include "inGalaxyModule/starNavigator.hpp"
#include "inGalaxyModule/starManager.hpp"
#include "tools/utility.hpp"
#include "tools/s_texture.hpp"
#include "tools/log.hpp"
#include "tools/ThreadPool.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "inGalaxyModule/Star3DWrapper.hpp"

#include "EntityCore/EntityCore.hpp"
#include "tools/context.hpp"

static float magnitude_max = 6.5;

#define DELTA_PARSEC 0.005

//////////////////// PARAMETRES STARS //////////////////////////////////////////
static float fov_stars = 60.f;
static float fov_factor = 108064.73f / (fov_stars*fov_stars); //30.017979
static float objectSizeLimit = 0;
static float min_rmag = 0.346891f;
static double fov= 180.;
////////////////////////////////////////////////////////////////////////////////

std::map<int,std::string> StarNavigator::common_names_map;
std::map<int,std::string> StarNavigator::common_names_map_i18n;
std::map<std::string,int> StarNavigator::common_names_index;
std::map<std::string,int> StarNavigator::common_names_index_i18n;

// ===========================================================================
//
//  Class StarNavigator
//
// ===========================================================================

StarNavigator::StarNavigator() : nbStars(0)
{
	starMgr = std::make_unique<StarManager>();

	createSC_context();
	starTexture = new s_texture("star16x16.png",TEX_LOAD_TYPE_PNG_SOLID,false);  // Load star texture no mipmap
	set->bindTexture(starTexture->getTexture(), 0);
	old_pos = v3fNull;

	fader.setDuration(3000);
	pool= new ThreadPool(std::thread::hardware_concurrency());

	computeRCMagTable();
}

void StarNavigator::createSC_context()
{
	VulkanMgr &vkmgr = *VulkanMgr::instance;
	Context &context = *Context::instance;

	m_dataGL = std::make_unique<VertexArray>(vkmgr);
	m_dataGL->createBindingEntry(7 * sizeof(float));
	m_dataGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // POS
	m_dataGL->addInput(VK_FORMAT_R32G32B32_SFLOAT); // COLOR
	m_dataGL->addInput(VK_FORMAT_R32_SFLOAT); // MAG
	layout = std::make_unique<PipelineLayout>(vkmgr);
	layout->setGlobalPipelineLayout(context.layouts.front().get());
	layout->setTextureLocation(0, &PipelineLayout::DEFAULT_SAMPLER);
	layout->setUniformLocation(VK_SHADER_STAGE_GEOMETRY_BIT, 1);
	layout->buildLayout();
	layout->build();
	pipeline = std::make_unique<Pipeline>(vkmgr, *context.render, PASS_MULTISAMPLE_DEPTH, layout.get());
	pipeline->setTopology(VK_PRIMITIVE_TOPOLOGY_POINT_LIST);
	pipeline->setBlendMode(BLEND_ADD);
	pipeline->bindVertex(*m_dataGL);
	pipeline->bindShader("starNav.vert.spv");
	pipeline->bindShader("starNav.geom.spv");
	pipeline->bindShader("starNav.frag.spv");
	pipeline->build();
	set = std::make_unique<Set>(vkmgr, *context.setMgr, layout.get());
	uMat = std::make_unique<SharedBuffer<Mat4f>>(*context.uniformMgr);
	set->bindUniform(uMat, 1);
	context.cmdInfo.commandBufferCount = 3;
	vkAllocateCommandBuffers(vkmgr.refDevice, &context.cmdInfo, cmds);
	for (int i = 0; i < 3; ++i)
        context.frame[i]->setName(cmds[i], "StarNav " + std::to_string(i));
	drawData = std::make_unique<SharedBuffer<VkDrawIndirectCommand>>(*context.tinyMgr);
	*drawData = VkDrawIndirectCommand{0, 1, 0, 0};
}


void StarNavigator::loadRawData(const std::string &fileName) noexcept
{
	starMgr->loadStarRaw(fileName);
	starMgr->loadStarBinCatalog(fileName);
	setListGlobalStarVisible();
}

void StarNavigator::loadOtherData(const std::string &fileName) noexcept
{
	starMgr->loadOtherStar(fileName);
	setListGlobalStarVisible();
}

void StarNavigator::loadData(const std::string &fileName, bool binaryData) noexcept
{
	if (binaryData)
		starMgr->loadStarBinCatalog(fileName);
	else
		starMgr->loadStarCatalog(fileName);
	setListGlobalStarVisible();
}

//! Load common names from file
int StarNavigator::loadCommonNames(const std::string& commonNameFile)
{
	common_names_map.clear();
	common_names_map_i18n.clear();
	common_names_index.clear();
	common_names_index_i18n.clear();

	cLog::get()->write("Loading star names from " + commonNameFile);

	FILE *cnFile=fopen(commonNameFile.c_str(),"r");
	if (!cnFile) {
		std::cerr << "Warning " << commonNameFile << " not found." << std::endl;
		return 0;
	}

	// Assign names to the matching stars, now support spaces in names
	char line[256];
	while (fgets(line, sizeof(line), cnFile)) {
		line[sizeof(line)-1] = '\0';
		unsigned int hip;
		if (sscanf(line,"%u",&hip)!=1) {
			std::cerr << "ERROR: StarMgr::loadCommonNames(" << commonNameFile << "): bad line: \"" << line << '"' << std::endl;
			return 0;
		}
		unsigned int i = 0;
		while (line[i]!='|' && i<sizeof(line)-2) ++i;
		i++;
		std::string englishCommonName =  line+i;
		// remove newline
		englishCommonName.erase(englishCommonName.length()-1, 1);

		// remove underscores
		for (std::string::size_type j=0; j<englishCommonName.length(); ++j) {
			if (englishCommonName[j]=='_') englishCommonName[j]=' ';
		}
		const std::string commonNameI18n = _(englishCommonName.c_str());
		std::string commonNameI18n_cap = commonNameI18n;
		transform(commonNameI18n.begin(), commonNameI18n.end(), commonNameI18n_cap.begin(), ::toupper);

		common_names_map[hip] = englishCommonName;
		common_names_index[englishCommonName] = hip;
		common_names_map_i18n[hip] = commonNameI18n;
		common_names_index_i18n[commonNameI18n_cap] = hip;

	}
	fclose(cnFile);
	return 1;
}

//! Update i18 names from english names according to passed translator.
//! The translation is done using gettext with translated strings defined in translations.h
void StarNavigator::updateI18n(Translator& trans)
{
	common_names_map_i18n.clear();
	common_names_index_i18n.clear();
	for (std::map<int,std::string>::iterator it(common_names_map.begin()); it!=common_names_map.end(); it++) {
		const int i = it->first;
		const std::string t(trans.translateUTF8(it->second));
		common_names_map_i18n[i] = t;
		std::string t_cap = t;
		transform(t.begin(), t.end(), t_cap.begin(), ::toupper);
		common_names_index_i18n[t_cap] = i;
	}
}


void StarNavigator::saveData(const std::string &fileName, bool binaryData) noexcept
{
	if (binaryData)
		starMgr->saveStarBinCatalog(fileName);
	else
		starMgr->saveStarCatalog(fileName);
}

StarNavigator::~StarNavigator()
{
	delete pool;
}

void StarNavigator::clearBuffer()
{
	nbStars = 0;
	starVec = (float *) Context::instance->transfer->beginPlanCopy(maxStars * 7 * sizeof(float));
}

starInfo* StarNavigator::getStarInfo(unsigned int HIPName) const {
	return starMgr->findStar(HIPName);
}

std::string StarNavigator::getStarName(unsigned int HIPName) {
	std::string name = common_names_map_i18n[HIPName];
	if (!name.empty())
		return name;
	else
		return std::to_string(HIPName);
}

void StarNavigator::setListGlobalStarVisible()
{
	std::vector<HyperCube*> hcList = starMgr->getHyperCubeList();
	std::vector<HyperCube*> hcGlobalVisible;
	std::vector<Cube*> cubeGlobalVisible;
	std::vector<HyperCube*> hcVisible;
	std::vector<Cube*> cubeVisible;

	for(std::vector<HyperCube*>::iterator i = hcList.begin(); i != hcList.end(); ++i) {
		HyperCube *hc = *i;

		hcGlobalVisible.push_back(hc);
		std::vector<Cube*> cubeList = hc->getCubeList();

		for(std::vector<Cube*>::iterator j = cubeList.begin(); j != cubeList.end(); ++j) {
			Cube *c = *j;

			cubeGlobalVisible.push_back(c);
			std::vector<starInfo*> stars = c->getStarList();

			for(std::vector<starInfo*>::iterator k = stars.begin(); k != stars.end(); ++k) {
				starInfo *si = *k;
				listGlobalStarVisible.push_back(si);
			}
		}
	}
	maxStars = listGlobalStarVisible.size();
	build();
}

Vec3f StarNavigator::color_table[128] = {
	Vec3f(0.780392,0.866666,1),
	Vec3f(0.788235,0.870588,1),
	Vec3f(0.8,0.878431,1),
	Vec3f(0.807843,0.882352,1),
	Vec3f(0.815686,0.886274,1),
	Vec3f(0.82745,0.894117,1),
	Vec3f(0.835294,0.898039,1),
	Vec3f(0.843137,0.905882,1),
	Vec3f(0.854901,0.909803,1),
	Vec3f(0.866666,0.917647,1),
	Vec3f(0.874509,0.921568,1),
	Vec3f(0.886274,0.92549,1),
	Vec3f(0.894117,0.933333,1),
	Vec3f(0.905882,0.941176,1),
	Vec3f(0.917647,0.945098,1),
	Vec3f(0.92549,0.952941,1),
	Vec3f(0.937254,0.956862,1),
	Vec3f(0.949019,0.964705,1),
	Vec3f(0.956862,0.968627,1),
	Vec3f(0.968627,0.97647,1),
	Vec3f(0.980392,0.980392,1),
	Vec3f(0.988235,0.988235,1),
	Vec3f(1,0.992156,0.996078),
	Vec3f(1,0.988235,0.984313),
	Vec3f(1,0.980392,0.972549),
	Vec3f(1,0.97647,0.960784),
	Vec3f(1,0.972549,0.952941),
	Vec3f(1,0.968627,0.941176),
	Vec3f(1,0.964705,0.929411),
	Vec3f(1,0.960784,0.917647),
	Vec3f(1,0.956862,0.905882),
	Vec3f(1,0.949019,0.898039),
	Vec3f(1,0.945098,0.886274),
	Vec3f(1,0.941176,0.878431),
	Vec3f(1,0.937254,0.866666),
	Vec3f(1,0.933333,0.854901),
	Vec3f(1,0.929411,0.843137),
	Vec3f(1,0.92549,0.835294),
	Vec3f(1,0.917647,0.823529),
	Vec3f(1,0.913725,0.815686),
	Vec3f(1,0.909803,0.803921),
	Vec3f(1,0.905882,0.796078),
	Vec3f(1,0.90196,0.784313),
	Vec3f(1,0.898039,0.77647),
	Vec3f(1,0.894117,0.764705),
	Vec3f(1,0.890196,0.752941),
	Vec3f(1,0.886274,0.745098),
	Vec3f(1,0.882352,0.737254),
	Vec3f(1,0.878431,0.72549),
	Vec3f(1,0.870588,0.713725),
	Vec3f(1,0.866666,0.705882),
	Vec3f(1,0.862745,0.694117),
	Vec3f(1,0.858823,0.686274),
	Vec3f(1,0.854901,0.674509),
	Vec3f(1,0.85098,0.666666),
	Vec3f(1,0.847058,0.654901),
	Vec3f(1,0.843137,0.643137),
	Vec3f(1,0.839215,0.635294),
	Vec3f(1,0.835294,0.62745),
	Vec3f(1,0.831372,0.615686),
	Vec3f(1,0.82745,0.607843),
	Vec3f(1,0.823529,0.596078),
	Vec3f(1,0.819607,0.588235),
	Vec3f(1,0.815686,0.57647),
	Vec3f(1,0.811764,0.568627),
	Vec3f(1,0.807843,0.556862),
	Vec3f(1,0.803921,0.545098),
	Vec3f(1,0.8,0.533333),
	Vec3f(1,0.796078,0.529411),
	Vec3f(1,0.792156,0.517647),
	Vec3f(1,0.788235,0.505882),
	Vec3f(1,0.784313,0.494117),
	Vec3f(1,0.780392,0.486274),
	Vec3f(1,0.77647,0.474509),
	Vec3f(1,0.772549,0.458823),
	Vec3f(1,0.768627,0.45098),
	Vec3f(1,0.764705,0.439215),
	Vec3f(1,0.760784,0.423529),
	Vec3f(1,0.756862,0.415686),
	Vec3f(1,0.752941,0.4),
	Vec3f(1,0.749019,0.392156),
	Vec3f(1,0.745098,0.37647),
	Vec3f(1,0.741176,0.360784),
	Vec3f(1,0.737254,0.349019),
	Vec3f(1,0.733333,0.329411),
	Vec3f(1,0.729411,0.313725),
	Vec3f(1,0.729411,0.30196),
	Vec3f(1,0.721568,0.278431),
	Vec3f(1,0.717647,0.258823),
	Vec3f(1,0.717647,0.235294),
	Vec3f(1,0.709803,0.215686),
	Vec3f(1,0.709803,0.196078),
	Vec3f(1,0.70196,0.164705),
	Vec3f(1,0.70196,0.141176),
	Vec3f(1,0.698039,0.109803),
	Vec3f(1,0.690196,0.062745),
	Vec3f(1,0.690196,0),
	Vec3f(1,0.686274,0),
	Vec3f(1,0.682352,0),
	Vec3f(1,0.678431,0),
	Vec3f(1,0.678431,0),
	Vec3f(1,0.674509,0),
	Vec3f(1,0.670588,0),
	Vec3f(1,0.666666,0),
	Vec3f(1,0.666666,0),
	Vec3f(1,0.662745,0),
	Vec3f(1,0.658823,0),
	Vec3f(1,0.654901,0),
	Vec3f(1,0.654901,0),
	Vec3f(1,0.65098,0),
	Vec3f(1,0.647058,0),
	Vec3f(1,0.643137,0),
	Vec3f(1,0.643137,0),
	Vec3f(1,0.639215,0),
	Vec3f(1,0.635294,0),
	Vec3f(1,0.631372,0),
	Vec3f(1,0.631372,0),
	Vec3f(1,0.62745,0),
	Vec3f(1,0.62745,0),
	Vec3f(1,0.623529,0),
	Vec3f(1,0.619607,0),
	Vec3f(1,0.615686,0),
	Vec3f(1,0.611764,0),
	Vec3f(1,0.611764,0),
	Vec3f(1,0.607843,0),
	Vec3f(1,0.603921,0),
	Vec3f(1,0.6,0),
	Vec3f(1,0.6,0)
};


void StarNavigator::computeRCMagTable()
{
	//calculation of light intensity and radius
	ToneReproductor *eye = new ToneReproductor();
	eye->setWorldAdaptationLuminance(4.721604);

	float mag=-4.;
	for(int i=0; i<256; i++) {
		computeRCMag(mag, eye, rc_mag_table+2*i);
		mag=mag+0.05;
	}
	delete eye;

	needComputeRCMagTable = false;
}


int StarNavigator::computeRCMag(float mag, const ToneReproductor *eye, float rc_mag[2])
{
	// code from hip_star_mgr
	if (mag > max_mag) {
		rc_mag[0] = rc_mag[1] = 0.f;
		return -1;
	}

	// rmag:
	rc_mag[0] = std::sqrt(
	                eye->adaptLuminance(
	                    std::exp(-0.92103f*(mag + mag_shift + 12.12331f)) * fov_factor))
	            * 30.f;

	if (rc_mag[0] < min_rmag) {
		rc_mag[0] = rc_mag[1] = 0.f;
		return -1;
	}

	// if size of star is too small (blink) we put its size to 1.2 --> no more blink
	// And we compensate the difference of brighteness with cmag
	if (rc_mag[0]<1.2f) {
		if (rc_mag[0] * starScale < 0.1f) {
			rc_mag[0] = rc_mag[1] = 0.f;
			return -1;
		}
		rc_mag[1] = rc_mag[0] * rc_mag[0] / 1.44f;
		if (rc_mag[1] * starMagScale < 0.1f) {
			rc_mag[0] = rc_mag[1] = 0.f;
			return -1;
		}
		rc_mag[0] = 1.2f;
	} else {
		// cmag:
		rc_mag[1] = 1.f;
		if (rc_mag[0] > starSizeLimit) {
			rc_mag[0] = starSizeLimit;
		}
	}
	// Global scaling
	rc_mag[0] *= starScale;
	rc_mag[1] *= starMagScale;
	return 0;
}

//pos indicates the position of the camera
void StarNavigator::computePosition(Vec3f posI) noexcept
{
	if (listGlobalStarVisible.size()<1)
		return;

	pos=Mat4f::xrotation(M_PI_2+23.4392803055555555556*M_PI/180)*posI;

	if (needComputeRCMagTable) {
		this->computeRCMagTable();
	} else {
		if ((pos-old_pos).length() < DELTA_PARSEC) { //proximity test
			return; //nothing to do.
		}
	}

	old_pos = pos;
	clearBuffer();

	unsigned int nbPaquets= 40;
	for(unsigned int i=0; i<nbPaquets; i++)
		this->results.emplace_back( pool->enqueue( StarNavigator::threadWrapper, this, maxStars*i/nbPaquets, maxStars*(i+1)/nbPaquets) );

	for(auto && result: results)
		result.get();
	results.clear();

	Context::instance->transfer->endPlanCopy(vertex->get(), nbStars * 7 * sizeof(float));
	drawData->get().vertexCount = nbStars;
}

void StarNavigator::build()
{
	Context &context = *Context::instance;
	vertex.reset();
	vertex = m_dataGL->createBuffer(0, maxStars * 7, context.globalBuffer.get());
	for (int i = 0; i < 3; ++i) {
		auto cmd = cmds[i];
		context.frame[i]->begin(cmd, PASS_MULTISAMPLE_DEPTH);
		pipeline->bind(cmd);
		layout->bindSets(cmd, {*context.uboSet, *set});
		vertex->bind(cmd);
		vkCmdDrawIndirect(cmd, drawData->getBuffer().buffer, drawData->getOffset(), 1, 0);
		context.frame[i]->compile(cmd);
	}
}

bool StarNavigator::computeChunk(unsigned int first, unsigned int last)
{
	float rayon;
	int indice;
	float intensite;

	for(unsigned int i = first; i != last; ++i) {
		starInfo *si = listGlobalStarVisible[i];

		float x = -si->posXYZ[0];
		float y = si->posXYZ[1];
		float z = si->posXYZ[2];

		//test magnitude if magnitude too low, the star will not be displayed
		float dist =sqrt((x-pos[0])*(x-pos[0]) + (y-pos[1])*(y-pos[1]) +(z-pos[2])*(z-pos[2]));
		float mag_v = si->mag+5*(log10(dist)-1);
		if ( mag_v  < magnitude_max) {

			//calculation of the radius and luminous intensity
			mag_v= round(mag_v*1000)/1000;
			indice = (int)((mag_v-(-4.0))/0.05);

			if (indice<0)
				indice=0;

			if (indice>256-1)
				indice=255;

			rayon =rc_mag_table[2*indice];
			intensite = rc_mag_table[2*indice+1];

			if (intensite <0.01)
				continue;

			// Output of hip_star_mgr::drawStar
			float magC = 2.f*rayon;
			// Roll off star size limit as fov decreases to match planet halo scale
			RangeMap<float> rmap(180, 1, -starSizeLimit, -(starSizeLimit + objectSizeLimit));
			float rolloff = -rmap.Map(fov);
			if( magC > rolloff )
				magC = rolloff;
			//END

			if (magC <0.2)
				continue;


			//Determination of the color
			Vec3f tcolor = color_table[si->B_V]*intensite ;

			// There must be no concurrent access to the same vulkan memory
			accessTab.lock();
			++nbStars;
			*(starVec++) = x;
			*(starVec++) = y;
			*(starVec++) = z;
			memcpy(starVec, (float *) tcolor, 3 * sizeof(float));
			starVec += 3;
			*(starVec++) = magC/2;
			accessTab.unlock();
		}
	}
	return true;
}


std::vector<ObjectBaseP> StarNavigator::searchAround(Vec3d v, double limitFov, const Navigator *nav)
{
	std::vector<ObjectBaseP> result;
	if (!getFlagStars())
		return result;
	v.normalize();
	limitFov = limitFov * (M_PI/180.);
	double cosLimitFov = cos(limitFov);
	for (auto &star: listGlobalStarVisible) {
		auto tmp = nav->helioToEarthPosEqu(Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180) * star->posXYZ);
		tmp[0] = -tmp[0];
		tmp.normalize();
		float dotProduct = tmp.dot(v);
		if (dotProduct > cosLimitFov)
			result.push_back(new Star3DWrapper(star, pos));
	}
	return result;
}

void StarNavigator::drawStarName(const Projector* prj)
{
	for (auto const& token : starNameToDraw) {
		prj->printGravity180(font, std::get<0>(token), std::get<1>(token), std::get<2>(token), std::get<3>(token), 4,4);
	}
	//std::cout << "Number of the names to print : " << starNameToDraw.size() << "\n";
}

void StarNavigator::draw(const Navigator * nav, const Projector* prj, bool scaling) noexcept
{
	if (starsFader==false)
		return;

	if (nbStars<1)
		return;

	starNameToDraw.clear();

	Mat4f matrix;
	if (scaling)
		matrix = Mat4f::scaling(1e-6) * nav->getHelioToEyeMat().convert() * Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180);
	else
		matrix = nav->getHelioToEyeMat().convert() * Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180);
	drawRaw(matrix);

	if (names_fader.isZero())
		return;

	const float names_brightness = fader.getInterstate() * names_fader;

	for (auto &s: listGlobalStarVisible) {
		Vec3f spos(-s->posXYZ[0], s->posXYZ[1], s->posXYZ[2]);
		float dist = (spos - pos).length();
		float mag_v = s->mag+5*(log10(dist)-1);

		if (mag_v < maxMagStarName) {
			const std::string starname = getStarName(s->HIP);
			if (!starname.empty()) {

				if (scaling)
					spos = nav->helioToEarthPosEqu(Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180)*spos)*1E-6;
				else
					spos = nav->helioToEarthPosEqu(Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180)*spos);
				Vec3d screenposd;
				prj->projectEarthEqu(spos, screenposd);

				Vec4f Color(HipStarMgr::color_table[s->B_V][0]*0.75,
							HipStarMgr::color_table[s->B_V][1]*0.75,
							HipStarMgr::color_table[s->B_V][2]*0.75,
							names_brightness);
				starNameToDraw.push_back(std::make_tuple(screenposd[0],screenposd[1], starname, Color));
			}
		}
	}
	this->drawStarName(prj);
}

void StarNavigator::drawRaw(const Mat4f &matrix) const noexcept
{
	*uMat = matrix;
	const int idx = Context::instance->frameIdx;
	Context::instance->frame[idx]->toExecute(cmds[idx], PASS_MULTISAMPLE_DEPTH);
}
