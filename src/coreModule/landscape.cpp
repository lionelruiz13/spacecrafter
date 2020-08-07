/*
 * Spacecrafter astronomy simulation and visualization
 *
 * Copyright (C) 2003 Fabien Chereau
 * Copyright (C) 2009 Digitalis Education Solutions, Inc.
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
 * Spacecrafter is a free open project of the LSS team
 * See the TRADEMARKS file for free open project usage requirements.
 *
 */

#include "coreModule/landscape.hpp"
#include "tools/init_parser.hpp"
#include "tools/log.hpp"
#include "tools/app_settings.hpp"
#include "tools/s_texture.hpp"
#include "atmosphereModule/tone_reproductor.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"

#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"
#include "tools/Renderer.hpp"


int Landscape::slices = 20;
int Landscape::stacks = 10;

std::unique_ptr<shaderProgram> Landscape::shaderLandscape;

std::unique_ptr<shaderProgram> Fog::shaderFog;
s_texture* Fog::fog_tex;

Landscape::Landscape(float _radius) : radius(_radius), sky_brightness(1.)
{
	map_tex = nullptr;
	map_tex_night = nullptr;

	valid_landscape = 0;
	cLog::get()->write( "Landscape generic created" , LOG_TYPE::L_INFO);
	haveNightTex = false;

	fog =nullptr;
	fog = new Fog(0.95f);
	assert(fog!=nullptr);
}

Fog::Fog(float _radius) : radius(_radius)
{
	m_fogGL = std::make_unique<VertexArray>();
	m_fogGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	m_fogGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);
}

void Landscape::createSC_context()
{
	shaderLandscape = std::make_unique<shaderProgram>();
	shaderLandscape->init("landscape2T.vert", "landscape2T.geom","landscape2T.frag");

	shaderLandscape->setUniformLocation("sky_brightness");
	shaderLandscape->setUniformLocation("fader");
	shaderLandscape->setUniformLocation("ModelViewMatrix");

	shaderLandscape->setSubroutineLocation(GL_FRAGMENT_SHADER,"withNightTex");
	shaderLandscape->setSubroutineLocation(GL_FRAGMENT_SHADER,"withoutNightTex");

	Fog::createSC_context();
}

void Landscape::setSkyBrightness(float b) {
	sky_brightness = b;
	fog->setSkyBrightness(b);
}

//! Set whether fog is displayed
void Landscape::fogSetFlagShow(bool b) {
	fog->setFlagShow(b);
}
//! Get whether fog is displayed
bool Landscape::fogGetFlagShow() const {
	return fog->getFlagShow();
}

void Landscape::update(int delta_time) {
	land_fader.update(delta_time);
	fog->update(delta_time);
}

void Fog::createSC_context()
{
	shaderFog = std::make_unique<shaderProgram>();
	shaderFog-> init( "fog.vert","fog.frag");

	shaderFog->setUniformLocation("fader");
	shaderFog->setUniformLocation("sky_brightness");
	shaderFog->setUniformLocation("ModelViewMatrix");

	fog_tex = new s_texture("fog.png",TEX_LOAD_TYPE_PNG_SOLID_REPEAT,false);
}

void Fog::initShader()
{
	std::vector<float> dataTex;
	std::vector<float> dataPos;

	createFogMesh(radius, radius*sinf(fog_alt_angle*M_PI/180.) , 128,1, &dataTex, &dataPos);

	m_fogGL->fillVertexBuffer(BufferType::POS3D, dataPos);
	m_fogGL->fillVertexBuffer(BufferType::TEXTURE, dataTex);

	dataTex.clear();
	dataPos.clear();
}

Fog::~Fog()
{}

Landscape::~Landscape()
{
	if (fog) delete fog;
}

Landscape* Landscape::createFromFile(const std::string& landscape_file, const std::string& section_name)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	std::string s;
	s = pd.getStr(section_name, "type");
	Landscape* ldscp = nullptr;
	if (s=="spherical") {
		ldscp = new LandscapeSpherical();
	} else if (s=="fisheye") {
		ldscp = new LandscapeFisheye();
	} else {
		cLog::get()->write( "Unknown landscape type: " + s  , LOG_TYPE::L_ERROR);
		// to avoid making this a fatal error, will load as a basic Landscape
		ldscp = new Landscape();
	}
	ldscp->load(landscape_file, section_name);
	return ldscp;
}

// create landscape from parameters passed in a hash (same keys as with ini file)
Landscape* Landscape::createFromHash(stringHash_t & param)
{
	// night landscape textures for spherical and fisheye landscape types
	std::string night_tex="";
	if (param["night_texture"] != "") 
		night_tex = param["path"] + param["night_texture"];

	std::string texture="";
	if (param["texture"] == "")
		texture = param["path"] + param["maptex"];
	else
		texture = param["path"] + param["texture"];

	bool mipmap; // Default on
	if (param["mipmap"] == "on" || param["mipmap"] == "1")
		mipmap = true;
	else
		mipmap = false;

	// NOTE: textures should be full filename (and path)
	if (param["type"]=="fisheye") {
		LandscapeFisheye* ldscp = new LandscapeFisheye();
		ldscp->create(param["name"], texture, Utility::strToDouble(param["fov"], Utility::strToDouble(param["texturefov"], 180)),
		              Utility::strToDouble(param["rotate_z"], 0.), night_tex, mipmap);
		return ldscp;
	} else if (param["type"]=="spherical") {
		LandscapeSpherical* ldscp = new LandscapeSpherical();
		ldscp->create(param["name"], texture, Utility::strToDouble(param["base_altitude"], -90),
		              Utility::strToDouble(param["top_altitude"], 90), Utility::strToDouble(param["rotate_z"], 0.),  night_tex, mipmap);
		return ldscp;
	} else {  //wrong Landscape
		Landscape* ldscp = new Landscape();
		cLog::get()->write( "Unknown landscape type in createFromHash: " + param["name"], LOG_TYPE::L_ERROR);
		return ldscp;
	}
}

// Load attributes common to all landscapes
void Landscape::loadCommon(const std::string& landscape_file, const std::string& section_name)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	name = pd.getStr(section_name, "name");
	author = pd.getStr(section_name, "author");
	description = pd.getStr(section_name, "description");

	fog->setAltAngle(pd.getDouble(section_name, "fog_alt_angle", 30.));
	fog->setAngleShift(pd.getDouble(section_name, "fog_angle_shift", 0.));

	if (name.empty()) {
		cLog::get()->write( "No valid landscape definition found for section " + section_name +" in file " + landscape_file , LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	} else {
		valid_landscape = 1;
	}
}

std::string Landscape::getFileContent(const std::string& landscape_file)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	std::string result;
	for (int i=0; i<pd.getNsec(); i++) {
		result += pd.getSecname(i) + '\n';
	}
	return result;
}


std::string Landscape::getLandscapeNames(const std::string& landscape_file)
{
	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);
	std::string result;
	for (int i=0; i<pd.getNsec(); i++) {
		result += pd.getStr(pd.getSecname(i), "name") + '\n';
	}
	return result;
}


void Landscape::draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav)
{
	if (!valid_landscape) return;
	if (!land_fader.getInterstate()) return;

	// Normal transparency mode
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	StateGL::enable(GL_CULL_FACE);
	StateGL::enable(GL_BLEND);

	shaderLandscape->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, map_tex->getID());

	if (haveNightTex && sky_brightness > 0.25) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, map_tex_night->getID());
		shaderLandscape->setSubroutine(GL_FRAGMENT_SHADER, "withNightTex");
	} else
		shaderLandscape->setSubroutine(GL_FRAGMENT_SHADER, "withoutNightTex");

	shaderLandscape->setUniform("sky_brightness",fmin(sky_brightness,1.0));
	shaderLandscape->setUniform("fader",land_fader.getInterstate());

	Mat4f matrix = (nav->getLocalToEyeMat() * Mat4d::zrotation(-rotate_z)).convert();
	shaderLandscape->setUniform("ModelViewMatrix",matrix);

	Renderer::drawArrays(shaderLandscape.get(), m_landscapeGL.get(), GL_TRIANGLE_STRIP,0,nbVertex);

	StateGL::disable(GL_CULL_FACE);
	StateGL::disable(GL_BLEND);

	glActiveTexture(GL_TEXTURE0);

	fog->draw(eye,prj,nav);
}


// Draw the horizon fog
void Fog::draw(ToneReproductor * eye, const Projector* prj, const Navigator* nav) const
{
	if (!fader.getInterstate()) return;

	StateGL::BlendFunc(GL_ONE, GL_ONE);

	StateGL::enable(GL_BLEND);
	StateGL::enable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	shaderFog->use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fog_tex->getID());
	shaderFog->setUniform("fader", fader.getInterstate());
	shaderFog->setUniform("sky_brightness", sky_brightness);
	Mat4f matrix = (nav->getLocalToEyeMat() * Mat4d::translation(Vec3d(0.,0.,radius*sinf(fog_angle_shift*M_PI/180.)))).convert();
	shaderFog->setUniform("ModelViewMatrix",matrix);

	Renderer::drawArrays(shaderFog.get(), m_fogGL.get(), GL_TRIANGLE_STRIP,0,nbVertex);

	glCullFace(GL_BACK);
	StateGL::disable(GL_CULL_FACE);
	glActiveTexture(GL_TEXTURE0);
}

void Landscape::deleteMapTex()
{
	if (map_tex) delete map_tex;
	map_tex = nullptr;
	if (map_tex_night) delete map_tex_night;
	map_tex_night = nullptr;
}

// *********************************************************************
//
// Fisheye landscape
//
// *********************************************************************

LandscapeFisheye::LandscapeFisheye(float _radius) : Landscape(_radius)
{
	rotate_z = 0;
}


LandscapeFisheye::~LandscapeFisheye()
{
	deleteMapTex();
}

void LandscapeFisheye::load(const std::string& landscape_file, const std::string& section_name)
{
	loadCommon(landscape_file, section_name);

	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);

	std::string type = pd.getStr(section_name, "type");
	if (type != "fisheye") {
		cLog::get()->write( "type mismatch for landscape " + section_name + ": expected fisheye found " + type, LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	std::string texture = pd.getStr(section_name, "texture");
	if (texture.empty()) {
		cLog::get()->write( "No texture for landscape " + section_name , LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	texture = AppSettings::Instance()->getLandscapeDir() + texture;

	std::string night_texture = pd.getStr(section_name, "night_texture", "");
	if (! night_texture.empty())
		night_texture = AppSettings::Instance()->getLandscapeDir() +night_texture;

	create(name, texture,
	       pd.getDouble(section_name, "fov", pd.getDouble(section_name, "texturefov", 180)),
	       pd.getDouble(section_name, "rotate_z", 0.),
	       night_texture,
	       pd.getBoolean(section_name, "mipmap", true));
}


// create a fisheye landscape from basic parameters (no ini file needed)
void LandscapeFisheye::create(const std::string _name, const std::string _maptex, double _texturefov,
                              const float _rotate_z, const std::string _maptex_night, bool _mipmap)
{
	valid_landscape = 1;  // assume ok...
	haveNightTex = false;
	cLog::get()->write( "Landscape Fisheye " + _name + " created" , LOG_TYPE::L_INFO);
	name = _name;
	map_tex = new s_texture(_maptex,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);

	if (! _maptex_night.empty()) {
		map_tex_night = new s_texture(_maptex_night,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);
		haveNightTex = true;
	}
	tex_fov = _texturefov*M_PI/180.;
	rotate_z = _rotate_z*M_PI/180.;

	initShader();
	fog->initShader();
}

void LandscapeFisheye::initShader()
{
	nbVertex = 2*slices*stacks + 2* stacks;
	GLfloat *datatex = new float[nbVertex*2];
	GLfloat *datapos = new float[nbVertex*3];

	createFisheyeMesh(radius,slices,stacks, tex_fov, datatex, datapos);

	m_landscapeGL = std::make_unique<VertexArray>();
	m_landscapeGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	m_landscapeGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);

	m_landscapeGL->fillVertexBuffer(BufferType::POS3D, nbVertex*3, datapos);
	m_landscapeGL->fillVertexBuffer(BufferType::TEXTURE, nbVertex*2, datatex);

	if (datatex) delete datatex;
	if (datapos) delete datapos;
}


static inline double FisheyeTexCoordFastS(double rho_div_fov, double costheta, double sintheta)
{
	if (rho_div_fov>0.5) rho_div_fov=0.5;
	return 0.5 + rho_div_fov * costheta;
}


static inline double FisheyeTexCoordFastT(double rho_div_fov, double costheta, double sintheta)
{
	if (rho_div_fov>0.5) rho_div_fov=0.5;
	return 0.5 + rho_div_fov * sintheta;
}

void LandscapeFisheye::createFisheyeMesh(double radius, int slices, int stacks, double texture_fov, GLfloat * datatex, GLfloat * datapos)
{
	unsigned int indice1=0;
	unsigned int indice3=0;
	double rho,x,y,z;
	int i, j;

	int nbr=0;
	const double drho = M_PI / stacks;
	double cos_sin_rho[2*(stacks+1)];
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		const double rho = i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const double dtheta = 2.0 * M_PI / slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		const double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triangle fan on texturing (s coord. at top/bottom tip varies)
	const int imax = stacks;

	// draw intermediate stacks as quad strips
	for (i = 0,cos_sin_rho_p=cos_sin_rho,rho=0.0; i < imax; ++i,cos_sin_rho_p+=2,rho+=drho) {

		for (j=0,cos_sin_theta_p=cos_sin_theta; j<= slices; ++j,cos_sin_theta_p+=2) {
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = cos_sin_rho_p[2];
			if (z>=0) {
				nbr++;
				z=z-0.01; //TODO magic number to export

				datatex[indice1] = FisheyeTexCoordFastS((rho + drho)/texture_fov,cos_sin_theta_p[0],-cos_sin_theta_p[1]);
				indice1++;
				datatex[indice1] = FisheyeTexCoordFastT((rho + drho)/texture_fov,cos_sin_theta_p[0],-cos_sin_theta_p[1]);
				indice1++;
				datapos[indice3]= x*radius;
				indice3++;
				datapos[indice3]= y*radius;
				indice3++;
				datapos[indice3]= z*radius;
				indice3++;
			}
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = cos_sin_rho_p[0];
			if (z>=0) {
				nbr++;
				z=z-0.01; //TODO magic number to export

				datatex[indice1] = FisheyeTexCoordFastS(rho/M_PI, cos_sin_theta_p[0], -cos_sin_theta_p[1]);
				indice1++;
				datatex[indice1] = FisheyeTexCoordFastT(rho/M_PI, cos_sin_theta_p[0], -cos_sin_theta_p[1]);
				indice1++;

				datapos[indice3]= x*radius;
				indice3++;
				datapos[indice3]= y*radius;
				indice3++;
				datapos[indice3]= z*radius;
				indice3++;
			}
		}
	}
	nbVertex = nbr;
}

// *********************************************************************
//
// spherical panoramas
//
// *********************************************************************

LandscapeSpherical::LandscapeSpherical(float _radius) : Landscape(_radius),  base_altitude(-90), top_altitude(90)
{
	rotate_z = 0;
}

LandscapeSpherical::~LandscapeSpherical()
{
	deleteMapTex();
}

void LandscapeSpherical::load(const std::string& landscape_file, const std::string& section_name)
{
	loadCommon(landscape_file, section_name);

	InitParser pd;	// The landscape data ini file parser
	pd.load(landscape_file);

	std::string type = pd.getStr(section_name, "type");
	if (type != "spherical" ) {
		cLog::get()->write( "Type mismatch for landscape " + section_name +", expected spherical, found " + type , LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}

	std::string texture = pd.getStr(section_name, "texture");
	if (texture.empty()) {
		cLog::get()->write( "No texture for landscape " + section_name , LOG_TYPE::L_ERROR);
		valid_landscape = 0;
		return;
	}
	texture = AppSettings::Instance()->getLandscapeDir() + texture;

	std::string night_texture = pd.getStr(section_name, "night_texture", "");
	if (! night_texture.empty())
		night_texture = AppSettings::Instance()->getLandscapeDir() +night_texture;

	create(name, texture,
	       pd.getDouble(section_name, "base_altitude", -90),
	       pd.getDouble(section_name, "top_altitude", 90),
	       pd.getDouble(section_name, "rotate_z", 0.),
	       night_texture,
	       pd.getBoolean(section_name, "mipmap", true));
}


// create a spherical landscape from basic parameters (no ini file needed)
void LandscapeSpherical::create(const std::string _name, const std::string _maptex, const float _base_altitude,
                                const float _top_altitude, const float _rotate_z, const std::string _maptex_night, bool _mipmap)
{
	valid_landscape = 1;  // assume ok...
	cLog::get()->write( "Landscape Spherical " + _name + " created" , LOG_TYPE::L_INFO);
	name = _name;
	map_tex = new s_texture(_maptex,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);

	if (!_maptex_night.empty()) {
		map_tex_night = new s_texture(_maptex_night,TEX_LOAD_TYPE_PNG_ALPHA,_mipmap);
		haveNightTex = true;
	} else
		haveNightTex = false;

	base_altitude = ((_base_altitude >= -90 && _base_altitude <= 90) ? _base_altitude : -90);
	top_altitude = ((_top_altitude >= -90 && _top_altitude <= 90) ? _top_altitude : 90);
	rotate_z = _rotate_z*M_PI/180.;

	initShader();
	fog->initShader();
}

void LandscapeSpherical::initShader()
{
	nbVertex = 2*slices*stacks + 2* stacks;

	GLfloat *datatex = new float[nbVertex*2];
	GLfloat *datapos = new float[nbVertex*3];

	createSphericalMesh(radius, 1.0, slices,stacks, base_altitude, top_altitude, datatex, datapos);

	m_landscapeGL = std::make_unique<VertexArray>();
	m_landscapeGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::STATIC);
	m_landscapeGL->registerVertexBuffer(BufferType::TEXTURE, BufferAccess::STATIC);

	m_landscapeGL->fillVertexBuffer(BufferType::POS3D, nbVertex*3, datapos);
	m_landscapeGL->fillVertexBuffer(BufferType::TEXTURE, nbVertex*2, datatex);

	if (datatex) delete[] datatex;
	if (datapos) delete[] datapos;
}


void LandscapeSpherical::createSphericalMesh(double radius, double one_minus_oblateness, int slices, int stacks,
        double bottom_altitude, double top_altitude , GLfloat * datatex, GLfloat * datapos)
{
	unsigned int indiceTex=0;
	unsigned int indicePos=0;

	double bottom = M_PI / 180. * bottom_altitude;
	double angular_height = M_PI / 180. * top_altitude - bottom;

	float x, y, z;
	float s, t;
	int i, j;
	t=0.0; // from inside texture is reversed

	const float drho = angular_height / (float) stacks;
	double cos_sin_rho[2*(stacks+1)];
	double *cos_sin_rho_p = cos_sin_rho;
	for (i = 0; i <= stacks; i++) {
		double rho = M_PI_2 + bottom + i * drho;
		*cos_sin_rho_p++ = cos(rho);
		*cos_sin_rho_p++ = sin(rho);
	}

	const float dtheta = 2.0 * M_PI / (float) slices;
	double cos_sin_theta[2*(slices+1)];
	double *cos_sin_theta_p = cos_sin_theta;
	for (i = 0; i <= slices; i++) {
		double theta = (i == slices) ? 0.0 : i * dtheta;
		*cos_sin_theta_p++ = cos(theta);
		*cos_sin_theta_p++ = sin(theta);
	}

	// texturing: s goes from 0.0/0.25/0.5/0.75/1.0 at +y/+x/-y/-x/+y axis
	// t goes from -1.0/+1.0 at z = -radius/+radius (linear along longitudes)
	// cannot use triane fan on texturing (s coord. at top/bottom tip varies)
	const float ds = 1.0 / slices;
	const float dt = -1.0 / stacks; // from inside texture is reversed

	// draw intermediate  as quad strips
	for (i = 0,cos_sin_rho_p = cos_sin_rho; i < stacks; i++,cos_sin_rho_p+=2) {
		s = 0.0;
		for (j = 0,cos_sin_theta_p = cos_sin_theta; j <= slices; j++,cos_sin_theta_p+=2) {
			x = -cos_sin_theta_p[1] * cos_sin_rho_p[1];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[1];
			z = -1.0 * cos_sin_rho_p[0];

			datatex[indiceTex++]=1-s;
			datatex[indiceTex++]=t;
			datapos[indicePos++]=x*radius;
			datapos[indicePos++]=y*radius;
			datapos[indicePos++]=z * one_minus_oblateness * radius;

			x = -cos_sin_theta_p[1] * cos_sin_rho_p[3];
			y = cos_sin_theta_p[0] * cos_sin_rho_p[3];
			z = -1.0 * cos_sin_rho_p[2];

			datatex[indiceTex++]=1-s;
			datatex[indiceTex++]=t-dt;
			datapos[indicePos++]=x*radius;
			datapos[indicePos++]=y*radius;
			datapos[indicePos++]=z * one_minus_oblateness * radius;

			s += ds;
		}
		t -= dt;
	}
}

void Fog::createFogMesh(GLdouble radius, GLdouble height, GLint slices, GLint stacks, std::vector<float>* dataTex, std::vector<float>* dataPos)
{
	nbVertex=0;
	GLdouble da, r, dz;
	GLfloat z ;
	GLint i;

	da = 2.0 * M_PI / slices;
	dz = height / stacks;

	GLfloat ds = 1.0 / slices;
	GLfloat dt = 1.0 / stacks;
	GLfloat t = 0.0;
	z = 0.0;
	r = radius;
	GLfloat s = 0.0;
	for (i = 0; i <= slices; i++) {
		GLfloat x, y;
		if (i == slices) {
			x = sinf(0.0);
			y = cosf(0.0);
		} else {
			x = sinf(i * da);
			y = cosf(i * da);
		}
		dataTex->push_back(s);
		dataTex->push_back(t);
		dataPos->push_back(x*r);
		dataPos->push_back(y*r);
		dataPos->push_back(z);
		nbVertex++;

		dataTex->push_back(s);
		dataTex->push_back(t+dt);
		dataPos->push_back(x*r);
		dataPos->push_back(y*r);
		dataPos->push_back(z+dz);
		nbVertex++;
		s += ds;
	}
}