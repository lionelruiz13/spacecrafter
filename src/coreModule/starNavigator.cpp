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
#include <unistd.h>

#include "coreModule/starNavigator.hpp"
#include "coreModule/starManager.hpp"
#include "tools/utility.hpp"
#include "tools/stateGL.hpp"
#include "tools/ThreadPool.hpp"
#include "tools/tone_reproductor.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/shader.hpp"
#include "tools/OpenGL.hpp"
#include "tools/Renderer.hpp"

static float magnitude_max = 6.5;

#define DELTA_PARSEC 0.005

//////////////////// PARAMETRES STARS //////////////////////////////////////////
static float fov_stars = 60.f;
static float fov_factor = 108064.73f / (fov_stars*fov_stars); //30.017979
static float objectSizeLimit = 0;
static float min_rmag = 0.346891f;
static double fov= 180.;
////////////////////////////////////////////////////////////////////////////////

// ===========================================================================
//
//  Class StarNavigator
//
// ===========================================================================

StarNavigator::StarNavigator()
{
	starMgr = nullptr;
	starMgr = new StarManager();

	createSC_context();
	starTexture = new s_texture("star16x16.png",TEX_LOAD_TYPE_PNG_SOLID,false);  // Load star texture no mipmap
	old_pos = v3fNull;
	
	pool= new ThreadPool(std::thread::hardware_concurrency());

	computeRCMagTable();
}

void StarNavigator::createSC_context()
{
	shaderStarNav = std::make_unique<shaderProgram>();
	shaderStarNav -> init("starNav.vert","starNav.geom","starNav.frag");
	shaderStarNav->setUniformLocation("Mat");

	m_dataGL = std::make_unique<VertexArray>();
	m_dataGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	m_dataGL->registerVertexBuffer(BufferType::COLOR, BufferAccess::STATIC);
	m_dataGL->registerVertexBuffer(BufferType::MAG, BufferAccess::STATIC);
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


void StarNavigator::saveData(const std::string &fileName, bool binaryData) noexcept
{
	std::cout << "Not implémented yet" << std::endl;
}

StarNavigator::~StarNavigator()
{
	delete pool;
	if (starMgr != nullptr)
		delete starMgr;
}

void StarNavigator::clearBuffer()
{
	starPos.clear();
	starColor.clear();
	starRadius.clear();
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
	//calcul de l'intensité lumineuse et du rayon
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
	// code issue de hip_star_mgr
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

//pos designe la position de la caméra
void StarNavigator::computePosition(Vec3f posI) noexcept
{
	if (listGlobalStarVisible.size()<1)
		return;

	pos=Mat4f::xrotation(M_PI_2+23.4392803055555555556*M_PI/180)*posI;

	if (needComputeRCMagTable) {
		this->computeRCMagTable();
	} else {
		if ((pos-old_pos).length() < DELTA_PARSEC) { //test de proximité
			return; //rien à faire.
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

	m_dataGL->fillVertexBuffer(BufferType::POS3D, starPos);
	m_dataGL->fillVertexBuffer(BufferType::COLOR, starColor);
	m_dataGL->fillVertexBuffer(BufferType::MAG, starRadius);
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

		//test magnitude si magnitude trop faible, l'étoile ne sera pas affichée
		float dist =sqrt((x-pos[0])*(x-pos[0]) + (y-pos[1])*(y-pos[1]) +(z-pos[2])*(z-pos[2]));
		float mag_v = si->mag+5*(log10(dist)-1);
		if ( mag_v  < magnitude_max) {

			//calcul du rayon et de l'intensité lumineuse
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

			// Issue de hip_star_mgr::drawStar
			float magC = 2.f*rayon;
			// Roll off star size limit as fov decreases to match planet halo scale
			RangeMap<float> rmap(180, 1, -starSizeLimit, -(starSizeLimit + objectSizeLimit));
			float rolloff = -rmap.Map(fov);
			if( magC > rolloff )
				magC = rolloff;
			//FIN

			if (magC <0.2)
				continue;


			//Détermination de la couleur
			Vec3f tcolor = color_table[si->B_V]*intensite ;

			//no simult acces to data
			accesTab.lock();
			insert_all(starPos, x, y, z);
			insert_vec3(starColor, tcolor);
			starRadius.push_back(magC/2);
			accesTab.unlock();
		}
	}
	return true;
}


void StarNavigator::draw(const Navigator * nav, const Projector* prj) const noexcept
{
	if (starsFader==false)
		return;

	if (starPos.size()<1)
		return;
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_ONE, GL_ONE);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, starTexture->getID());

	Mat4f matrix=nav->getHelioToEyeMat().convert();
	matrix=matrix*Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180);
	
	shaderStarNav->use();
	shaderStarNav->setUniform("Mat",matrix);

	// m_dataGL->bind();
	// glDrawArrays(GL_POINTS,0,starPos.size()/3);
	// m_dataGL->unBind();
	// shaderStarNav->unuse();
	Renderer::drawArrays(shaderStarNav.get(), m_dataGL.get(), GL_POINTS,0,starPos.size()/3);
}
