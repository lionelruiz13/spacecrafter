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

#define PARSEC_SPEED 30856775817672
#define AL            9460730472581
#define VLUM              299792458
#define CONV_PARSEC_AL 3.261563777459  // 1 al = CONV_PARSEC_AL pc
#define PLX_MIN 0.3
#define MAX_VALUE 99999

#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cmath>
#include "inGalaxyModule/starManager.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"

#include <list>
#include <cstdlib>
#include <cstring>

/**
 * The inputs and outputs of the binary files are of the form
 *
 * for hypercubes:
 * | Char c='H' | 3 floats posX, posY, posZ | 1 uint number (of cube) |
 *    1                          12                     4             = 17 octets
 *
 * For cubes:
 * | Char c='c' | 3 floats posX, posY, posZ | 1 uint number ( of stars) |
 *           1                     12                   4               = 17 octets
 *
 * For stars:
 * | Char c='s' | 1 uint  name HIP| 3floats posX, posY, posZ | 2floats pmRa, pmDe | float mag | int B_V | float pc(parsec)|
 *           1                  4                       12                8               4          4        4          = 37 octets
 */

unsigned int Cube::NbTotalCube = 0;

unsigned int HyperCube::NbTotalHyperCube = 0;

// ===========================================================================
//
//  Class Cube
//
// ===========================================================================

Cube::Cube( int  a, int b, int c):size(CUBESIZE)
{
	c_x=a;
	c_y=b;
	c_z=c;
	NbTotalCube ++;
	MinMagnitude = 500;
	//~ printf("creation of a cube of center (%i,%i,%i)\n", c_x, c_y, c_z);
}


Cube::~Cube()
{
}


void Cube::addStar(StarInfo &&si)
{
	if (si.mag < MinMagnitude)
		MinMagnitude = si.mag;
	starList.push_back(std::move(si));
}



// ===========================================================================
//
//  Class HyperCube
//
// ===========================================================================

HyperCube::HyperCube(int x, int y, int z) :hcSize(HCSIZE), min(9999), max(0)
{
	c_x= x;
	c_y= y;
	c_z= z;
	NbTotalHyperCube ++;
	MinMagnitude = 500;
	//~ printf("creation of a hypercube of center (%i,%i,%i)\n", c_x, c_y, c_z);
}

unsigned int HyperCube::getNbrStars()
{
	unsigned tmp = 0;
	for (Cube &cube : cubeList) {
		tmp += cube.getNbStars();
	}
	return tmp;
}

Cube &HyperCube::getCube(int a, int b, int c)
{
	for (Cube &cube : cubeList) {
		if ((cube.getCx() == a) && (cube.getCy() == b) && (cube.getCz() == c))
			return cube;
	}
	return cubeList.emplace_back(a, b, c);
}

//Find in which cube the star is located
void HyperCube::addCubeStar(StarInfo &&star)
{
	float X = star.posXYZ[0];
	float Y = star.posXYZ[1];
	float Z = star.posXYZ[2];

	int cubX = floor((X) / CUBESIZE);
	int cubY = floor((Y) / CUBESIZE);
	int cubZ = floor((Z) / CUBESIZE);

	int cub_centerX =  cubX * CUBESIZE;
	int cub_centerY =  cubY * CUBESIZE;
	int cub_centerZ =  cubZ * CUBESIZE;

	// if ( ( abs(cub_centerX-X) > CUBESIZE) ||  ( abs(cub_centerY-Y) > CUBESIZE) || ( abs(cub_centerZ-Z) > CUBESIZE) ) {
		// std::cout << "c entree " << X << " " << Y << " " << Z << std::endl;
		// std::cout << "c sortie " << cub_centerX << " " << cub_centerY << " " << cub_centerZ << std::endl;
		//~ sleep(2);
	// }

	getCube(cub_centerX, cub_centerY, cub_centerZ).addStar(std::move(star));
}

HyperCube::~HyperCube()
{
	//~ printf("Delete H...\n");
}

float HyperCube::getMinMagnitude()
{
	//if not computed then computes it
	if (MinMagnitude==500) {
		for (Cube &cube : cubeList) {
			if( cube.getMinMagnitude() <MinMagnitude  )
				MinMagnitude=cube.getMinMagnitude();
		}
	}
	return MinMagnitude;
}


// ===========================================================================
//
//  Class StarManager
//
// ===========================================================================

StarManager::StarManager()
{
}

StarManager::~StarManager()
{
}

unsigned int StarManager::getNbrStars()
{
	unsigned int tmp = 0;
	for (HyperCube &hc : hyperCubeList) {
		tmp += hc.getNbrStars();
	}
	return tmp;
}

// READING THE INTERNAL CATALOG
bool StarManager::loadStarCatalog(const std::string &fileName)
{
	//std::cout << "StarManager::loadStarCatalog " << fileName << std::endl;
	//cLog::get()->write("StarManager::loadStarCatalog " + fileName, LOG_TYPE::L_DEBUG);
	std::ifstream file(fileName, std::ifstream::in);
	if (!file.is_open()) {
		//std::cout << "ERROR: Unable to open the file " << fileName << std::endl;
		cLog::get()->write("StarManager::loadStar catalog unable to open" + fileName + " - Feature disabled", LOG_TYPE::L_ERROR);
		return false;
	}

	std::string obj;
	int cubesNumber;
	float hcX, hcY, hcZ;
	int starsNumber;
	float cubeX, cubeY, cubeZ;
	unsigned int HIP;
	float starX, starY, starZ;
	float pmRA, pmDE, mag, pc;
	int B_V;
	uint64_t nbrH=0, nbrC=0, nbrS=0;
	unsigned int numberRead = 0;

	std::string line; // variable which will contain each line of the file
	//std::cout << "Reading the catalog "  << fileName << std::endl;
	cLog::get()->write("Starmanager, loading text catalogue "+fileName);

	while (getline(file, line)) {
		//we start with a hypercube
		std::istringstream istrHc(line);
		istrHc >> obj >> hcX >> hcY >> hcZ >> cubesNumber;

		if (obj[0] != 'H') {
			//std::cout << "error parsing:  H needed but i see "<< line << std::endl;
			cLog::get()->write("StarManager error parsing:  H needed but i see "+line, LOG_TYPE::L_ERROR);
			return false;
		}

		HyperCube *hc = &getHC(hcX,hcY,hcZ);
		nbrH++;

		// we read each cube one after the other
		for(int i=0; i<cubesNumber; i++) {
			getline(file, line);
			std::istringstream istrC(line);
			istrC >> obj>> cubeX >> cubeY >> cubeZ >> starsNumber;
			if (obj[0] != 'C') {
				//std::cout << "error parsing:  C needed but i see "<< line << std::endl;
				cLog::get()->write("StarManager error parsing:  C needed but i see "+line, LOG_TYPE::L_ERROR);
				return false;
			}

			Cube *cube = &hc->getCube(cubeX,cubeY,cubeZ);
			nbrC++;

			// we read all the stars in the cube
			for(int i=0; i<starsNumber; i++) {
				getline(file, line);

				std::istringstream istrS(line);
				istrS >> obj>> HIP >> starX >> starY >> starZ >> pmRA >> pmDE >> mag >> B_V >> pc;

				if (obj[0] != 'S') {
					//std::cout << "error parsing:  S needed but i see "<< line << std::endl;
					cLog::get()->write("StarManager error parsing:  S needed but i see "+line, LOG_TYPE::L_ERROR);
					return false;
				}
				nbrS++;
				cube->addStar(StarInfo{
					.HIP=HIP,
					.posXYZ={starX,starY,starZ},
					.pmRA=pmRA,
					.pmDE=pmDE,
					.mag=mag,
					.pc=pc,
					.B_V=static_cast<uint8_t>(B_V),
					.show = true,
				});
				numberRead++;
			}
		}
	}
	file.close();

	std::ostringstream oss;
	oss << "HyperCubes : " << nbrH << std::endl;
	oss << "Cubes      : " << nbrC << std::endl;
	oss << "Stars      : " << nbrS;
	cLog::get()->write(oss.str());
	//std::cout << oss.str() << std::endl;
	return true;
}


// READING THE INTERNAL CATALOG
bool StarManager::loadStarBinCatalog(const std::string &fileName)
{
	//std::cout << "StarManager::loadStarBinCatalog " << fileName << std::endl;
	std::ifstream fileIn(fileName, std::ios::binary| std::ios::in);
	if (!fileIn.is_open()) {
		cLog::get()->write("StarManager, error opening file "+fileName, LOG_TYPE::L_WARNING);
		return false;
	}

	std::string obj;
	char c;
	int cubesNumber;
	float hcX, hcY, hcZ;
	int starsNumber;
	float cubeX, cubeY, cubeZ;
	unsigned int HIP;
	float starX, starY, starZ;
	float pmRA, pmDE, mag, pc;
	int B_V;
	uint64_t nbrH=0, nbrC=0, nbrS=0;

	std::string line; // variable which will contain each line of the file
	//std::cout << "Reading the catalog "  << fileName << std::endl;
	cLog::get()->write("Starmanager, loading bin catalogue "+fileName);

	while (!fileIn.eof()) {
		//we start with a hypercube
		if (!fileIn.get(c))
			break;

		if (c != 'H') {
			std::ostringstream oss;
			oss << "StarManager, error parsing " << std::endl;
			oss << "Position in file : " << fileIn.tellg() << std::endl;
			oss << "error parsing:  H needed but i see "<< c << std::endl;
			oss << "R HyperCubes : " << nbrH << std::endl;
			oss << "R Cubes      : " << nbrC << std::endl;
			oss << "R Stars      : " << nbrS << std::endl;
			cLog::get()->write(oss.str(),LOG_TYPE::L_ERROR);
			return false;
		}
		nbrH++;
		//~ cout << "H" << endl;

		fileIn.read((char *)&hcX,sizeof(hcX));
		fileIn.read((char *)&hcY,sizeof(hcY));
		fileIn.read((char *)&hcZ,sizeof(hcZ));
		HyperCube &hc = getHC(hcX,hcY,hcZ);
		fileIn.read((char *)&cubesNumber,sizeof(cubesNumber));

		// we read each cube one after the other
		for(int i=0; i<cubesNumber; i++) {

			fileIn.get(c);
			if (c != 'C') {
				std::ostringstream oss;
				oss << "StarManager, error parsing " << std::endl;
				oss << "Position in file : " << fileIn.tellg() << std::endl;
				oss << "error parsing:  C needed but i see "<< c << std::endl;
				oss << "R HyperCubes : " << nbrH << std::endl;
				oss << "R Cubes      : " << nbrC << std::endl;
				oss << "R Stars      : " << nbrS << std::endl;
				cLog::get()->write(oss.str(),LOG_TYPE::L_ERROR);
				return false;
			}
			nbrC++;
			//~ cout << "C";
			fileIn.read((char *)&cubeX,sizeof(cubeX));
			fileIn.read((char *)&cubeY,sizeof(cubeY));
			fileIn.read((char *)&cubeZ,sizeof(cubeZ));
			Cube &cube = hc.getCube(cubeX,cubeY,cubeZ);
			fileIn.read((char *)&starsNumber,sizeof(starsNumber));

			// we read all the stars in the cube
			for(int i=0; i<starsNumber; i++) {

				fileIn.get(c);
				if (c != 'S') {
					std::ostringstream oss;
					oss << "StarManager, error parsing " << std::endl;
					oss << "Position in file : " << fileIn.tellg() << std::endl;
					oss << "error parsing:  S needed but i see "<< c << std::endl;
					oss << "R HyperCubes : " << nbrH << std::endl;
					oss << "R Cubes      : " << nbrC << std::endl;
					oss << "R Stars      : " << nbrS << std::endl;
					cLog::get()->write(oss.str(),LOG_TYPE::L_ERROR);
					return false;
				}
				nbrS++;
				//~ cout << "S";

				fileIn.read((char *)&HIP,sizeof(HIP));
				fileIn.read((char *)&starX,sizeof(starX));
				fileIn.read((char *)&starY,sizeof(starY));
				fileIn.read((char *)&starZ,sizeof(starZ));
				fileIn.read((char *)&pmRA,sizeof(pmRA));
				fileIn.read((char *)&pmDE,sizeof(pmDE));
				fileIn.read((char *)&mag,sizeof(mag));
				fileIn.read((char *)&B_V,sizeof(B_V));
				fileIn.read((char *)&pc,sizeof(pc));

				cube.addStar(StarInfo{
					.HIP=HIP,
					.posXYZ={starX,starY,starZ},
					.pmRA=pmRA,
					.pmDE=pmDE,
					.mag=mag,
					.pc=pc,
					.B_V=static_cast<uint8_t>(B_V),
					.show = true,
				});
			}
		}
	}
	fileIn.close();

	std::ostringstream oss;
	oss << "HyperCubes : " << nbrH << std::endl;
	oss << "Cubes      : " << nbrC << std::endl;
	oss << "Stars      : " << nbrS;
	cLog::get()->write(oss.str());
	//std::cout << oss.str() << std::endl;
	return true;
}


bool StarManager::saveStarBinCatalog(const std::string &fileName)
{
	//std::cout << "StarManager::saveStarBinCatalog " << fileName << std::endl;
	cLog::get()->write("StarManager::saveStarBinCatalog " + fileName, LOG_TYPE::L_DEBUG);
	std::ofstream file(fileName, std::ios::binary| std::ios::out);
	std::cout.precision(6);
	float x,y,z, pmRA, pmDE, mag, pc;
	int nbr, B_V;

	if (!file.is_open()) {
		//std::cout << "Error writing saveStarCatalog" << std::endl;
		cLog::get()->write("Error writing saveStarCatalog", LOG_TYPE::L_ERROR);
		return false;
	}

	uint64_t nbrH=0, nbrC=0, nbrS=0;

	for (HyperCube &hc : hyperCubeList) {
		//~ file << "H" << " " << hc.getCx() << " " << hc.getCy() << " " << hc.getCz() << " " << hc.getNbrCubes() << std::endl;
		x = hc.getCx();
		y = hc.getCy();
		z = hc.getCz();
		nbr = hc.getNbrCubes();

		file.put('H');
		nbrH++;
		file.write((char *)&x, sizeof(x));
		file.write((char *)&y, sizeof(y));
		file.write((char *)&z, sizeof(z));
		file.write((char *)&nbr, sizeof(nbr));

		for (Cube &cube : hc) {
			//~ file << "C" << " " << (*c)->getCx() << " " <<(*c)->getCy() << " " << (*c)->getCz() << " " << (*c)->getNbStars() << std::endl;
			file.put('C');
			nbrC++;
			x = cube.getCx();
			y = cube.getCy();
			z = cube.getCz();
			nbr = cube.getNbStars();
			file.write((char *)&x, sizeof(x));
			file.write((char *)&y, sizeof(y));
			file.write((char *)&z, sizeof(z));
			file.write((char *)&nbr, sizeof(nbr));

			for (StarInfo &star : cube) {
				file.put('S');
				nbrS++;
				nbr = star.HIP;
				x = star.posXYZ[0];
				y = star.posXYZ[1];
				z = star.posXYZ[2];
				file.write((char *)&nbr, sizeof(nbr));
				file.write((char *)&x, sizeof(y));
				file.write((char *)&y, sizeof(y));
				file.write((char *)&z, sizeof(z));
				pmRA = star.pmRA;
				pmDE = star.pmDE;
				mag = star.mag;
				B_V = star.B_V;
				pc = star.pc;
				file.write((char *)&pmRA, sizeof(pmRA));
				file.write((char *)&pmDE, sizeof(pmDE));
				file.write((char *)&mag, sizeof(mag));
				file.write((char *)&B_V, sizeof(B_V));
				file.write((char *)&pc, sizeof(pc));
				if (file.bad())
					cLog::get()->write("Error writing", LOG_TYPE::L_ERROR);
					//std::cout << "error writing" << std::endl;

				//~ file << "S" << " " << (*star)->HIP << " " << (*star)->posXYZ[0] << " " << (*star)->posXYZ[1] << " " << (*star)->posXYZ[2] << " "
				//~ << (*star)->pmRA << " " << (*star)->pmDE << " " << (*star)->mag << " " << (*star)->B_V << " " << (*star)->pc << std::endl;
			}
		}
	}
	file.flush();
	file.close();
	return true;
}

bool StarManager::saveStarCatalog(const std::string &fileName)
{
	//std::cout << "StarManager::saveStarCatalog " << fileName << std::endl;
	cLog::get()->write("StarManager::saveStarCatalog " + fileName, LOG_TYPE::L_DEBUG);
	std::ofstream file(fileName);
	std::cout.precision(6);

	if (!file.is_open()) {
		//std::cout << "Error writing saveStarCatalog" << std::endl;
		cLog::get()->write("Error writing saveStarCatalog", LOG_TYPE::L_ERROR);
		return false;
	}

	for (HyperCube &hc : hyperCubeList) {
		file << "H" << " " << hc.getCx() << " " << hc.getCy() << " " << hc.getCz() << " " << hc.getNbrCubes() << std::endl;
		for (Cube &cube : hc) {
			file << "C" << " " << cube.getCx() << " " <<cube.getCy() << " " << cube.getCz() << " " << cube.getNbStars() << std::endl;
			for (StarInfo &star : cube) {
				file << "S" << " " << star.HIP << " " << star.posXYZ[0] << " " << star.posXYZ[1] << " " << star.posXYZ[2] << " " << star.pmRA << " " << star.pmDE << " " << star.mag << " " << star.B_V << " " << star.pc << std::endl;
			}
		}
	}

	file.close();
	return true;
}

HyperCube &StarManager::getHC(int a, int b, int c)
{
	for (HyperCube &hc : hyperCubeList) {
		if ((hc.getCx() == a) && (hc.getCy() == b) && (hc.getCz() == c))
			return hc;
	}
	return hyperCubeList.emplace_back(a, b, c);
}


//Find in which hypercube the star is located
void StarManager::addHcStar(StarInfo &&star)
{
	float X = star.posXYZ[0];
	float Y = star.posXYZ[1];
	float Z = star.posXYZ[2];

	int hcX = floor((X +HCSIZE/2) / HCSIZE);
	int hcY = floor((Y +HCSIZE/2) / HCSIZE);
	int hcZ = floor((Z +HCSIZE/2) / HCSIZE);

	int hc_centerX = hcX * HCSIZE;
	int hc_centerY = hcY * HCSIZE;
	int hc_centerZ = hcZ * HCSIZE;

	// if  ( (abs(hc_centerX-X)>HCSIZE/2) || (abs(hc_centerY-Y)>HCSIZE/2) || (abs(hc_centerZ-Z)>HCSIZE/2) ) {
	// 	std::cout << "hc input " << X << " " << Y << " " << Z << std::endl;
	// 	std::cout << "hc output " << hc_centerX << " " << hc_centerY << " " << hc_centerZ << std::endl;
		//~ sleep(2);
	// }

	getHC(hc_centerX, hc_centerY, hc_centerZ).addCubeStar(std::move(star));
}

// Star catalog reading function
// Fills a list of stars with those found in the catalog
//
//    I/311 Hipparcos, the New Reduction  (van Leeuwen, 2007)
//    Hipparcos, the new Reduction of the Raw data van Leeuwen F.
//    <Astron. Astrophys. 474, 653 (2007)>
//
//    http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/311
/*bool StarManager::loadStarRaw(const std::string &catPath)
{
	//std::cout << "StarManager::loadStarRaw " << catPath << std::endl;
	cLog::get()->write("Starmanager::loadStarRaw " + catPath);
	std::ifstream file(catPath);
	unsigned int hip;
	float RArad, DErad, Plx, pmRA, pmDE, mag_app, BV;
	unsigned starAccepted=0, starRejected =0;

	//Create the hypercube and the initial cube to contain the sun
	Vec3f origin(0.00001, 0.00001, 0.00001);
	StarInfo *sun = new StarInfo;
	sun->posXYZ = origin;
	sun->HIP = 0;
	sun->mag = 4.52649;
	sun->B_V = 38;
	sun->pc = 1.32484;

	HyperCube *hyper_Initial = new HyperCube(0,0,0);
	Cube *cube_Initial = new Cube(0,0,0);

	cube_Initial->addStar(sun);
	hyper_Initial->addCube(cube_Initial);
	addHyperCube(hyper_Initial);

	if (file) { // Fails if can't open the file
		std::string line1; // variable which will contain each line of the file
		cLog::get()->write("Reading the initial catalog " + catPath);

		// readig file line per line
		while (getline(file, line1)) {

			std::istringstream hip_iss(line1.substr(0,6));
			hip_iss >> hip;

			std::istringstream RArad_iss(line1.substr(15,12));
			RArad_iss >> RArad;

			std::istringstream DErad_iss(line1.substr(29,12));
			DErad_iss >> DErad;

			std::istringstream Plx_iss(line1.substr(44,6));
			Plx_iss >> Plx;

			std::istringstream pmRA_iss(line1.substr(52,7));
			pmRA_iss >> pmRA;

			std::istringstream pmDE_iss(line1.substr(61,7));
			pmDE_iss >> pmDE;

			std::istringstream mag_iss(line1.substr(129,7));
			mag_iss >> mag_app;

			std::istringstream BV_iss(line1.substr(153,5));
			BV_iss >> BV;

			StarInfo *si = nullptr;
			si = createStar( hip, RArad, DErad, Plx, pmRA, pmDE, mag_app, BV);
			this->addHcStar(si);
			starAccepted++;
		}
		file.close();

		cLog::get()->write("Star(s) accepted : " + std::to_string(starAccepted) );
		cLog::get()->write("Star(s) rejected : " + std::to_string(starRejected) );

		return true;
	} else {
		cLog::get()->write("StarManager, unable to open star cat", LOG_TYPE::L_ERROR);
		return false;
	}
}*/

// Star catalog reading function
// Fills a list of stars with those found in the catalog
//
//    I/311 Hipparcos, the New Reduction  (van Leeuwen, 2007)
//    Hipparcos, the new Reduction of the Raw data van Leeuwen F.
//    <Astron. Astrophys. 474, 653 (2007)>
//
//    http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/311
bool StarManager::loadStarRaw(const std::string &catPath)
{
	//std::cout << "StarManager::loadStarRaw " << catPath << std::endl;
	cLog::get()->write("Starmanager::loadStarRaw " + catPath);
	std::ifstream file(catPath);
	unsigned int hip;
	float RArad, DErad, Plx, pmRA, pmDE, mag_app, BV;
	unsigned starAccepted=0, starRejected =0;

	//Create the hypercube and the initial cube to contain the sun
	HyperCube *hyper_Initial = &getHC(0, 0, 0);
	Cube *cube_Initial = &hyper_Initial->getCube(0, 0, 0);
	cube_Initial->addStar(StarInfo{ // Sun
		.HIP = 0,
		.posXYZ = {0.00001, 0.00001, 0.00001},
		.mag = 4.52649,
		.pc = 1.32484,
		.B_V = 38,
	});

	if (file) { // Fails if can't open the file
		std::string line1; // variable which will contain each line of the file
		std::string section; // variable which will contain each section of the line
		cLog::get()->write("Reading the initial catalog " + catPath);

		// readig file line per line
		while (getline(file, line1)) {
			std::istringstream section_iss(line1);
			for (int nb_section = 0; getline(section_iss, section, ';'); nb_section++) {
				switch (nb_section) {
					case 0 : break;
					case 1 : break;
					case 2 :
					{
						std::istringstream hip_iss(section);
				   		hip_iss >> hip;
						// return true;
						break;
					}
					case 3 :
					{
						std::istringstream RArad_iss(section);
						RArad_iss >> RArad;
						break;
					}
					case 4 :
					{
						std::istringstream DErad_iss(section);
						DErad_iss >> DErad;
						break;
					}
					case 5 :
					{
						std::istringstream Plx_iss(section);
						Plx_iss >> Plx;
						break;
					}
					case 6 : break; // error plx
					case 7 :
					{
						std::istringstream pmRA_iss(section);
						pmRA_iss >> pmRA;
						break;
					}
					case 8 :
					{
						std::istringstream pmDE_iss(section);
						pmDE_iss >> pmDE;
						break;
					}
					case 9 :
					{
						std::istringstream mag_iss(section);
						mag_iss >> mag_app;
						break;
					}
					case 10 :
					{
						std::istringstream BV_iss(section);
						BV_iss >> BV;
						break;
					}
				}
			}
			addHcStar(StarInfo_create(hip, RArad*3.1415926/180.0, DErad*3.1415926/180.0, Plx, pmRA, pmDE, mag_app, BV));
			starAccepted++;
		}
		file.close();

		cLog::get()->write("Star(s) accepted : " + std::to_string(starAccepted) );
		cLog::get()->write("Star(s) rejected : " + std::to_string(starRejected) );

		return true;
	} else {
		cLog::get()->write("StarManager, unable to open star cat", LOG_TYPE::L_ERROR);
		return false;
	}
}


StarInfo StarInfo_create(unsigned int hip, float ra, float de, float plx, float pmRa, float pmDe, float mag, float bv)
{
	StarInfo ret;
	// conversion in x,y,z coordinates
	// we decide to modify the minimal Plx and to fix it at 0.2 which makes a star at worst at 16000 al
	double parsec = 1000.0 / ((plx >PLX_MIN) ? plx : (PLX_MIN+0.01* (rand()%10)));
	ret.posXYZ[0] = parsec * cos( pmRa ) * cos( pmDe );
	ret.posXYZ[1] = parsec * sin ( pmDe );
	ret.posXYZ[2] = parsec * sin ( pmRa ) * cos( pmDe );

	//patch Lionel Ruiz to conform to the OpenGL benchmark
	ret.posXYZ[0] = -ret.posXYZ[0];
	ret.posXYZ[1] = -ret.posXYZ[1];

	ret.HIP = hip;
	ret.pmRA = pmRa;
	ret.pmDE = pmDe;
	ret.pc = parsec;
	ret.mag = mag-5*(log10(parsec)-1);
	int b_v = (bv+0.5)/4.*127.;
	ret.show = true;

	if (b_v < 0) {
		ret.B_V = 0;
		// cout << ret.HIP << " " << si ->B_V << endl;
		cLog::get()->write("Star B_V to 0 with hip "+ std::to_string(ret.HIP), LOG_TYPE::L_WARNING);
	} else if (b_v > 127) {
		ret.B_V = 127;
		// cout << ret.HIP << " " << si ->B_V << endl;
		cLog::get()->write("Star B_V to 127 with hip "+ std::to_string(ret.HIP), LOG_TYPE::L_WARNING);
	} else {
		ret.B_V = static_cast<uint8_t>(b_v);
	}
	return ret;
}

int StarManager::getNbrCubes()
{
	nbrCubes = 0;
	for (HyperCube &hc : hyperCubeList) {
		nbrCubes += hc.getNbrCubes();
	}
	return nbrCubes;
}


float StarManager::getMinMagnitude()
{
	//if not calculated then calculate it
	if (MinMagnitude==500) {
		for (HyperCube &hc : hyperCubeList) {
			if( hc.getMinMagnitude() < MinMagnitude  )
				MinMagnitude=hc.getMinMagnitude();
		}
	}
	return MinMagnitude;
}


void StarManager::HyperCubeStatistiques()
{
	for(int i=0; i<NBR_PAS_STATHC; i++)
		statHc[i]=0;
	int tmp, tmp_max_cube;
	tmp_max_cube = HCSIZE / CUBESIZE * HCSIZE / CUBESIZE * HCSIZE / CUBESIZE;
	int un_cube =0, max_cube=0;
	int val = (HCSIZE/CUBESIZE)*(HCSIZE/CUBESIZE)*(HCSIZE/CUBESIZE)/ NBR_PAS_STATHC;
	for (HyperCube &hc : hyperCubeList) {
		tmp = hc.getNbrCubes();
		if (tmp==1) un_cube++;
		if (tmp==tmp_max_cube) max_cube++;
		tmp= tmp/val;
		if (tmp>=NBR_PAS_STATHC) //TODO really useful ?
			tmp = NBR_PAS_STATHC -1;
		statHc[tmp]=statHc[tmp]+1;
	}
	std::cout << std::endl;
	std::cout << "Statistics on the number of cubes in HyperCubes" << std::endl;
	std::cout << std::setw(12) << "Number : " << "Hypercubes concerned" << std::endl;
	for(int i=0; i<NBR_PAS_STATHC; i++) {
		std::cout << std::setw(3) << i*8 << " à " << std::setw(3) << (i+1)*8 << " : " << std::setw(12) << statHc[i] << std::endl;
	}
	std::cout << std::endl;
	std::cout << un_cube << " hc with only one cube to manage" << std::endl;
	std::cout << max_cube << " hc at max capacity" << std::endl;
}



void StarManager::MagStarStatistiques()
{
	for(int i=0; i< MAG_PAS; i++)
		statMagStars[i]=0;
	for (HyperCube &hc : hyperCubeList) {
		for (Cube &cube : hc) {
			for (StarInfo &star : cube) {
				int tmp=(40+star.mag)/5;
				if (tmp<0)
					tmp=0;
				if (tmp+1>MAG_PAS)
					tmp=MAG_PAS-1;
				++statMagStars[tmp];
			}
		}
	}
	std::cout << std::endl;
	std::cout << "Distribution of star magnitudes" << std::endl;
	std::cout << std::setw(9) << "Magnitude : " << "Number of stars" << std::endl;
	for(int i=0; i< MAG_PAS; i++)
		std::cout << std::setw(3) << i*5-40<< " à " << std::setw(3) << i*5-35 << " : " << std::setw(12) << statMagStars[i] << std::endl;
}


bool StarManager::verificationData()
{
	int nbr_cube_max = (HCSIZE/CUBESIZE)*(HCSIZE/CUBESIZE)*(HCSIZE/CUBESIZE);
	//verification of hypercubes
	for (HyperCube &hc : hyperCubeList) {
		if (hc.getCx() %HCSIZE !=0 || hc.getCy() %HCSIZE !=0 || hc.getCz() %HCSIZE !=0 ) {
			printf("HyperCube -- coordinates %i %i %i\n", hc.getCx(), hc.getCy(), hc.getCz());
			return false;
		}

		if (hc.getNbrCubes()<0 || hc.getNbrCubes()>nbr_cube_max) {
			printf("HyperCube -%i %i %i- numbers of cubes %i\n", hc.getCx(), hc.getCy(), hc.getCz(), hc.getNbrCubes());
			return false;
		}

		if (hc.getNbrCubes()==0) {
			printf("HyperCube -%i %i %i- no cube\n", hc.getCx(), hc.getCy(), hc.getCz());
		}

		//verification of cubes
		for (Cube &cube : hc) {
			if (cube.getCx() %CUBESIZE !=0 || cube.getCy() %CUBESIZE !=0 || cube.getCz() %CUBESIZE !=0 ) {
				printf("HyperCube -- coordinates %i %i %i\n", hc.getCx(), hc.getCy(), hc.getCz());
				printf("Cube -- coordinates %i %i %i\n", cube.getCx(), cube.getCy(), cube.getCz());
				return false;
			}

			if (cube.getNbStars()<0 || cube.getNbStars()>65000) {
				printf("HyperCube -%i %i %i-\n", hc.getCx(), hc.getCy(), hc.getCz());
				printf("Cube -- coordinates %i %i %i stars : %i\n", cube.getCx(), cube.getCy(), cube.getCz(), cube.getNbStars());
				return false;
			}

			if (cube.getNbStars()==0) {
				printf("HyperCube -%i %i %i-\n", hc.getCx(), hc.getCy(), hc.getCz());
				printf("Cube -- coordinates %i %i %i %i has no stars \n", cube.getCx(), cube.getCy(), cube.getCz(), cube.getNbStars());
			}
		}
	}
	return true;
}

//TODO the same in BINARY version
bool StarManager::saveAsterismStarsPosition(const std::string &fileNameIn,const std::string &fileNameOut)
{
	std::cout << "StarManager::saveAsterismStarsPosition " << fileNameIn << " " << fileNameOut << std::endl;
	std::ifstream fileIn(fileNameIn, std::ifstream::in);
	std::ofstream fileOut(fileNameOut, std::ifstream::out);
	std::list<int> asterimStars;
	unsigned int HIPName=0;
	StarInfo* si= nullptr;

	if (fileIn && fileOut) { // Fails if can't open the file
		std::string line; // variable which will contain each line of the file
		// cout << "Reading the catalog "  << fileNameIn << std::endl;
		cLog::get()->write("StarManager read AsterismCatalogue "+fileNameIn );
		cLog::get()->write("StarManager write AsterismCatalogue "+fileNameOut );

		while (getline(fileIn, line)) {
			si= nullptr;
			if (line !="") {
				HIPName = Utility::strToInt(line);
				si = findStar(HIPName);
				if (si != nullptr)
					fileOut << HIPName << " " << (*si).posXYZ[0] << " " << (*si).posXYZ[1] << " " << (*si).posXYZ[2] << std::endl;
				else
					cLog::get()->write("StarManager, AsterismCatalogue unknown "+ line, LOG_TYPE::L_WARNING );
			}
		}
		fileIn.close();
		fileOut.close();
		return true;
	}
	cLog::get()->write("StarManager, AsterismCatalogue, unable to read/write files", LOG_TYPE::L_ERROR);
	return false;
}

StarInfo *StarManager::findStar(unsigned int HIPName)
{
	for (HyperCube &hc : hyperCubeList) {
		for (Cube &cube : hc) {
			for (StarInfo &star : cube) {
				if (star.HIP == HIPName)
					return &star;
			}
		}
	}
	//printf("HIPName %i not found\n",HIPName);
	return nullptr;
}


bool StarManager::loadOtherStar(const std::string &fileName)
{
	std::cout << "StarManager::loadOtherStar " << fileName << std::endl;
	std::ifstream fileIn(fileName, std::ifstream::in);

	if (fileIn) { // Fails if can't open the file
		std::string line; // variable which will contain each line of the file
		// cout << "Reading the catalog "  << fileNameIn << std::endl;
		cLog::get()->write("StarManager add the file "+fileName );

		unsigned int hip;
		float ra, de, plx, pmRa, pmDe, mag, bv;

		while (getline(fileIn, line)) {

			if (line !="") {
				if (line[0]=='#')
					continue;

				std::istringstream parseLine(line);
				parseLine >> hip >> ra >> de >> plx >> pmRa >> pmDe >> mag >> bv;

				addHcStar(StarInfo_create(hip, ra, de, plx, pmRa, pmDe, mag, bv));
			}
		}
		fileIn.close();
		return true;
	}
	cLog::get()->write("StarManager, loadOtherStar, unable to read file "+ fileName, LOG_TYPE::L_ERROR);

	return false;
}
