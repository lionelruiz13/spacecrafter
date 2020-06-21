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
#include <string>
#include <unistd.h>

#include "coreModule/starLines.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include "coreModule/projector.hpp"
#include "navModule/navigator.hpp"
#include "tools/ia.hpp"
#include "tools/OpenGL.hpp"
#include "tools/shader.hpp"

StarLines::StarLines()
{
	createSC_context();
	lineColor =  Vec3f(1.0,1.0,0.0);
	isAlive = false;
}

StarLines::~StarLines()
{
	linePos.clear();
}

void StarLines::createSC_context()
{
	shaderStarLines = std::make_unique<shaderProgram>();
	shaderStarLines -> init("starLines.vert","starLines.geom", "starLines.frag");
	shaderStarLines->setUniformLocation({"Mat", "Color", "Fader"});
	
	m_dataGL = std::make_unique<VertexArray>();
	m_dataGL->registerVertexBuffer(BufferType::POS3D, BufferAccess::DYNAMIC);
}

bool StarLines::loadHipCatalogue(std::string fileName) noexcept
{
	std::ifstream fileIn(fileName.c_str());

	if (!fileIn.is_open()) {
		cLog::get()->write("StarLines error opening "+fileName, LOG_TYPE::L_ERROR);
		printf("StarLines error opening %s\n", fileName.c_str());
		return -1;
	}

	std::string record;
	int hip;
	float x,y,z;
	HIPpos tmp;

	while (!fileIn.eof() && std::getline(fileIn, record)) {

		if (record.size()!=0 && record[0]=='#')
			continue;

		std::istringstream istr(record);
		if (!(istr >> hip >> x >> y >> z)) {
			cLog::get()->write("StarLines error parsing HIP_data "+record, LOG_TYPE::L_ERROR);
			printf("StarLines error parsing HIP_data %s \n", record.c_str());
			return false;
		}
		tmp.first = hip;
		tmp.second = Vec3f(-x,y,z);
		HIP_data.push_back(tmp);
	}
	fileIn.close();
	cLog::get()->write("StarLines reading successfully cat "+fileName, LOG_TYPE::L_DEBUG);
	isAlive = true;
	return true;
}

bool StarLines::loadHipBinCatalogue(std::string fileName) noexcept
{
	std::ifstream fileIn(fileName.c_str(), std::ios::binary|std::ios::in );

	if (!fileIn.is_open()) {
		cLog::get()->write("StarLines error opening binary "+fileName, LOG_TYPE::L_ERROR);
		printf("StarLines error opening binary %s\n", fileName.c_str());
		return -1;
	}

	std::string record;
	int hip;
	float x,y,z;
	HIPpos tmp;
	char Ver[3];

	//lecture version
	fileIn.read((char *)&Ver,sizeof(Ver));
	//~ printf("%c %c %c\n", Ver[0], Ver[1], Ver[2]);

	//lecture des etoiles
	while (!fileIn.eof()) {

		fileIn.read((char *)&hip,sizeof(hip));
		fileIn.read((char *)&x,sizeof(x));
		fileIn.read((char *)&y,sizeof(y));
		fileIn.read((char *)&z,sizeof(z));

		tmp.first = hip;
		tmp.second = Vec3f(-x,y,z);
		HIP_data.push_back(tmp);
	}
	fileIn.close();
	cLog::get()->write("StarLines bin reading successfully cat "+fileName, LOG_TYPE::L_DEBUG);

	isAlive = true;
	return true;
}


bool StarLines::loadData(std::string fileName) noexcept
{
	std::ifstream fileIn(fileName.c_str());

	if (!fileIn.is_open()) {
		cLog::get()->write("StarLines error opening Data "+fileName, LOG_TYPE::L_ERROR);
		return false;
	}

	std::string record;
	linePos.clear();

	while (!fileIn.eof() && std::getline(fileIn, record)) {

		if (record.size()!=0 && record[0]=='#')
			continue;

		loadStringData(record);
	}
	cLog::get()->write("StarLines read Data successfully "+fileName, LOG_TYPE::L_ERROR);
	return true;
}


void StarLines::loadStringData(std::string record) noexcept
{
	unsigned int HIP1;
	unsigned int HIP2;
	Vec3f VNull(0.0,0.0,0.0);
	std::string abbreviation;
	unsigned int nb_segments=0;

	std::istringstream istr(record);
	if (!(istr >> abbreviation >> nb_segments)) {
		cLog::get()->write("StarLines error parsing line "+record, LOG_TYPE::L_ERROR);
		printf("StarLines error parsing\n");
		return;
	}

	for (unsigned int i=0; i<nb_segments; ++i) {
		HIP1 = 0;
		HIP2 = 0;
		istr >> HIP1 >> HIP2;

		if (HIP1==0 || HIP2==0) {
			cLog::get()->write("StarLines error parsing line ", LOG_TYPE::L_ERROR);
			printf("StarLines error parsing line\n");
			continue;
		}

		Vec3f tmp1 = searchInHip(HIP1);
		Vec3f tmp2 = searchInHip(HIP2);
		if (tmp1==VNull || tmp2 ==VNull) {
			if (tmp1==VNull) {
				printf("StarLines error parsing HIP %i not found\n", HIP1);
				cLog::get()->write("StarLines error parsing not found HIP " + Utility::intToString(HIP1), LOG_TYPE::L_ERROR);
			}
			if (tmp2==VNull) {
				printf("StarLines error parsing HIP %i not found\n", HIP2);
				cLog::get()->write("StarLines error parsing not found HIP " + Utility::intToString(HIP2), LOG_TYPE::L_ERROR);
			}
			continue;
		} else {
			// SEGMENT THE LINES
			int nblines=10;
			for(int j=0; j<nblines ; j++) {
				linePos.push_back(tmp1[0]*(nblines-j)/nblines+tmp2[0]*j/nblines);
				linePos.push_back(tmp1[1]*(nblines-j)/nblines+tmp2[1]*j/nblines);
				linePos.push_back(tmp1[2]*(nblines-j)/nblines+tmp2[2]*j/nblines);
				linePos.push_back(tmp1[0]*(nblines-(j+1))/nblines+tmp2[0]*(j+1)/nblines);
				linePos.push_back(tmp1[1]*(nblines-(j+1))/nblines+tmp2[1]*(j+1)/nblines);
				linePos.push_back(tmp1[2]*(nblines-(j+1))/nblines+tmp2[2]*(j+1)/nblines);
			}
		}
	}
	m_dataGL->fillVertexBuffer(BufferType::POS3D, linePos);
}


void StarLines::drop()
{
	linePos.clear();
}

Vec3f StarLines::searchInHip(int HIP)
{
	for (std::vector<HIPpos>::iterator it = HIP_data.begin() ; it != HIP_data.end(); ++it) {
		if ((*it).first == HIP) {
			return (*it).second;
		}
	}
	return v3fNull;
}

//version 3D in GALAXY mode
void StarLines::draw(const Navigator * nav) noexcept
{
	//commun aux deux fonctions
	if (!isAlive) return;
	if (linePos.size()<2)
		return;
	if (!showFader.getInterstate() ) return;

	//paramétrage des matrices pour opengl4
	Mat4f matrix=nav->getHelioToEyeMat().convert();
	matrix=matrix*Mat4f::xrotation(-M_PI_2-23.4392803055555555556*M_PI/180);

	this->drawGL(matrix);
}

//version 2D, in SOLAR_SYSTEM MODE
void StarLines::draw(const Projector* prj) noexcept
{
	//commun aux deux fonctions
	if (!isAlive) return;
	if (linePos.size()<2)
		return;
	if (!showFader.getInterstate() ) return;

	//paramétrage des matrices pour opengl4
	Mat4f matrix= prj-> getMatJ2000ToEye()*Mat4f::xrotation(-M_PI_2);

	this->drawGL(matrix);
}


//version 3D
void StarLines::drawGL(Mat4f & matrix)  noexcept
{
	StateGL::enable(GL_BLEND);
	StateGL::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Normal transparency mode

	shaderStarLines->use();
	shaderStarLines->setUniform("Mat",matrix);
	shaderStarLines->setUniform("Color",lineColor);
	shaderStarLines->setUniform("Fader", showFader.getInterstate() );

	m_dataGL->bind();
	glDrawArrays(GL_LINES,0,linePos.size()/3);
	m_dataGL->unBind();

	shaderStarLines->unuse();
}
