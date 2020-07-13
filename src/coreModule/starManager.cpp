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
#include "coreModule/starManager.hpp"
#include "tools/utility.hpp"
#include "tools/log.hpp"
#include <unistd.h>
#include <list>
#include <cstdlib>

/**
 * Les entrées sorties des fichiers binaires sont de la forme
 * 
 * pour les hypercubes:
 * | Char c='H' | 3 floats posX, posY, posZ | 1 uint number (of cube) |
 *    1                          12                     4             = 17 octets
 * 
 * Pour les cubes:
 * | Char c='c' | 3 floats posX, posY, posZ | 1 uint number ( of stars) |
 *           1                     12                   4               = 17 octets
 * 
 * Pour les étoiles:
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
	//~ printf("création d'un cube de centre (%i,%i,%i)\n", c_x, c_y, c_z);
}


Cube::~Cube()
{
	while(!starList.empty()) {
		delete starList.back();
		starList.pop_back();
	}
}


void Cube::addStar(starInfo *si)
{
	starList.push_back(si);
	if ((si->mag) < MinMagnitude)
		MinMagnitude = si->mag;
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
	nbrCubes=0;
	//~ printf("création d'un hypercube de centre (%i,%i,%i)\n", c_x, c_y, c_z);
}

void HyperCube::addCube(Cube *c)
{
	cubeList.push_back( c );
	nbrCubes=nbrCubes+1;
}

unsigned int HyperCube::getNbrStars()
{
	unsigned tmp = 0;
	std::vector<Cube*> list = getCubeList();
	for(std::vector<Cube*>::iterator i = list.begin(); i!= list.end(); ++i) {
		tmp = tmp+ (*i)->getNbStars();
	}
	return tmp;
}

//Détermine si un cube existe, si oui retourne un pointeur
Cube* HyperCube::cubeExist(int a, int b, int c)
{
	std::vector<Cube*> list = getCubeList();
	for(std::vector<Cube*>::iterator i = list.begin(); i!= list.end(); ++i) {
		Cube *cube = *i;
		if( ( (*i)->getCx() == a) && ( (*i)->getCy() == b) && ( (*i)->getCz() == c) ) {
			return cube;
		}
	}
	return nullptr;
}

//Trouver dans quelle Cube se trouve l'étoile
void HyperCube::addCubeStar(starInfo* star)
{
	float X = star->posXYZ[0];
	float Y = star->posXYZ[1];
	float Z = star->posXYZ[2];

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

	Cube *tmp = nullptr;
	tmp= cubeExist(cub_centerX, cub_centerY, cub_centerZ);

	if ( tmp == nullptr) { //retourne un pointeur null ni le cube n'existe pas
		tmp = new Cube(cub_centerX, cub_centerY, cub_centerZ); //Création d'un cube
		addCube(tmp);
		tmp->addStar(star);
	} else {
		tmp->addStar(star);
	}
}

HyperCube::~HyperCube()
{
	//~ printf("Suppression H...\n");
	while(!cubeList.empty()) {
		delete cubeList.back();
		cubeList.pop_back();
	}
}

float HyperCube::getMinMagnitude()
{
	//si pas calculé alors calcule le
	if (MinMagnitude==500) {
		for(std::vector<Cube*>::iterator i = cubeList.begin(); i!= cubeList.end(); ++i) {
			if( (*i)->getMinMagnitude() <MinMagnitude  )
				MinMagnitude=(*i)->getMinMagnitude();
		}
	}
	return MinMagnitude;
}


// ===========================================================================
//
//  Class StarManager
//
// ===========================================================================

StarManager::StarManager():nbrCubes(0), nbrHyperCubes(0), MinMagnitude(500)
{
	for(int i=0; i<NBR_PAS_STATHC; i++) statHc[i]=0;
}

void StarManager::addHyperCube(HyperCube *hcb)
{
	hyperCubeList.push_back( hcb );
	nbrHyperCubes++;
}

StarManager::~StarManager()
{
	//~ printf("suppression ...\n");
	while(!hyperCubeList.empty()) {
		delete hyperCubeList.back();
		hyperCubeList.pop_back();
	}
}


unsigned int StarManager::getNbrStars()
{
	unsigned int tmp = 0;
	std::vector<HyperCube*> list = getHyperCubeList();
	for(std::vector<HyperCube*>::iterator i = list.begin(); i!= list.end(); ++i) {
		tmp = tmp + (*i)->getNbrStars();
	}
	return tmp;
}

// LECTURE DU CATALOGUE INTERNE
bool StarManager::loadStarCatalog(const std::string &fileName)
{
	std::cout << "StarManager::loadStarCatalog " << fileName << std::endl;
	std::ifstream file(fileName, std::ifstream::in);
	if (!file.is_open()) {
		std::cout << "ERREUR: Impossible d'ouvrir le fichier " << fileName << std::endl;
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
	long unsigned int nbrH=0, nbrC=0, nbrS=0;
	unsigned int numberRead = 0;

	std::string line; // variable which will contain each line of the file
	std::cout << "Lecture du catalogue "  << fileName << std::endl;
	cLog::get()->write("Starmanager, loading catalogue "+fileName);

	while (getline(file, line)) {
		//on commence par un hypercube
		std::istringstream istrHc(line);
		istrHc >> obj >> hcX >> hcY >> hcZ >> cubesNumber;

		if (obj[0] != 'H') {
			std::cout << "error parsing:  H needed but i see "<< line << std::endl;
			return false;
		}

		HyperCube *hc = new HyperCube(hcX,hcY,hcZ);
		nbrH++;

		// on lit chaque cube les un après les autres
		for(int i=0; i<cubesNumber; i++) {
			getline(file, line);
			std::istringstream istrC(line);
			istrC >> obj>> cubeX >> cubeY >> cubeZ >> starsNumber;
			if (obj[0] != 'C') {
				std::cout << "error parsing:  C needed but i see "<< line << std::endl;
				return false;
			}

			Cube *cube = new Cube(cubeX,cubeY,cubeZ);
			nbrC++;

			// on lit toutes les etoiles dans le cube
			for(int i=0; i<starsNumber; i++) {
				getline(file, line);

				std::istringstream istrS(line);
				istrS >> obj>> HIP >> starX >> starY >> starZ >> pmRA >> pmDE >> mag >> B_V >> pc;

				if (obj[0] != 'S') {
					std::cout << "error parsing:  S needed but i see "<< line << std::endl;
					return false;
				}

				Vec3f xyz(starX,starY,starZ);
				starInfo *si = new starInfo();
				si->HIP=HIP;
				si->posXYZ=xyz;
				si->pmRA=pmRA;
				si->pmDE=pmDE;
				si->mag=mag;
				si->B_V=B_V;
				si->pc=pc;
				nbrS++;

				cube->addStar(si);
				numberRead++;
			}
			hc->addCube(cube); //TODO et si le nombre de cube est dépassé ?
		}
		addHyperCube(hc);
	}
	file.close();

	std::ostringstream oss;
	oss << "HyperCubes : " << nbrH << std::endl;
	oss << "Cubes      : " << nbrC << std::endl;
	oss << "Stars      : " << nbrS;
	cLog::get()->write(oss.str());
	std::cout << oss.str() << std::endl;
	return true;
}


// LECTURE DU CATALOGUE INTERNE
bool StarManager::loadStarBinCatalog(const std::string &fileName)
{
	std::cout << "StarManager::loadStarBinCatalog " << fileName << std::endl;
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
	long unsigned int nbrH=0, nbrC=0, nbrS=0;

	std::string line; // variable which will contain each line of the file
	std::cout << "Lecture du catalogue "  << fileName << std::endl;
	cLog::get()->write("Starmanager, loading catalogue "+fileName);

	while (!fileIn.eof()) {
		//on commence par un hypercube
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
		HyperCube *hc = new HyperCube(hcX,hcY,hcZ);
		fileIn.read((char *)&cubesNumber,sizeof(cubesNumber));

		// on lit chaque cube les un après les autres
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
			Cube *cube = new Cube(cubeX,cubeY,cubeZ);
			fileIn.read((char *)&starsNumber,sizeof(starsNumber));

			// on lit toutes les etoiles dans le cube
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

				Vec3f xyz(starX,starY,starZ);
				starInfo *si = new starInfo();
				si->HIP=HIP;
				si->posXYZ=xyz;
				si->pmRA=pmRA;
				si->pmDE=pmDE;
				si->mag=mag;
				si->B_V=B_V;
				si->pc=pc;

				cube->addStar(si);
			}
			hc->addCube(cube); //TODO et si le nombre de cube est dépassé ?
		}
		addHyperCube(hc);
	}
	fileIn.close();
	
	std::ostringstream oss;
	oss << "HyperCubes : " << nbrH << std::endl;
	oss << "Cubes      : " << nbrC << std::endl;
	oss << "Stars      : " << nbrS;
	cLog::get()->write(oss.str());
	std::cout << oss.str() << std::endl;
	return true;
}


bool StarManager::saveStarBinCatalog(const std::string &fileName)
{
	std::cout << "StarManager::saveStarBinCatalog " << fileName << std::endl;	
	std::ofstream file(fileName, std::ios::binary| std::ios::out);
	std::cout.precision(6);
	float x,y,z, pmRA, pmDE, mag, pc;
	int nbr, B_V;

	if (!file.is_open()) {
		std::cout << "Error writing saveStarCatalog" << std::endl;
		return false;
	}

	long unsigned  int nbrH=0, nbrC=0, nbrS=0;

	for(std::vector<HyperCube*>::iterator hc = hyperCubeList.begin(); hc!= hyperCubeList.end(); hc++) {

		//~ file << "H" << " " << (*hc)->getCx() << " " << (*hc)->getCy() << " " << (*hc)->getCz() << " " << (*hc)->getNbrCubes() << std::endl;
		x = (*hc)->getCx();
		y = (*hc)->getCy();
		z = (*hc)->getCz();
		nbr = (*hc)->getNbrCubes();

		file.put('H');
		nbrH++;
		file.write((char *)&x, sizeof(x));
		file.write((char *)&y, sizeof(y));
		file.write((char *)&z, sizeof(z));
		file.write((char *)&nbr, sizeof(nbr));

		std::vector<Cube*> List = (*hc)->getCubeList();
		for(std::vector<Cube*>::iterator c = List.begin(); c!= List.end(); ++c) {

			//~ file << "C" << " " << (*c)->getCx() << " " <<(*c)->getCy() << " " << (*c)->getCz() << " " << (*c)->getNbStars() << std::endl;
			file.put('C');
			nbrC++;
			x = (*c)->getCx();
			y = (*c)->getCy();
			z = (*c)->getCz();
			nbr = (*c)->getNbStars();
			file.write((char *)&x, sizeof(x));
			file.write((char *)&y, sizeof(y));
			file.write((char *)&z, sizeof(z));
			file.write((char *)&nbr, sizeof(nbr));

			std::vector<starInfo*> List2 = (*c)->getStarList();
			for(std::vector<starInfo*>::iterator star = List2.begin(); star!= List2.end(); ++star) {
				file.put('S');
				nbrS++;
				nbr = (*star)->HIP;
				x = (*star)->posXYZ[0];
				y = (*star)->posXYZ[1];
				z = (*star)->posXYZ[2];
				file.write((char *)&nbr, sizeof(nbr));
				file.write((char *)&x, sizeof(y));
				file.write((char *)&y, sizeof(y));
				file.write((char *)&z, sizeof(z));
				pmRA = (*star)->pmRA;
				pmDE = (*star)->pmDE;
				mag = (*star)->mag;
				B_V = (*star)->B_V;
				pc = (*star)->pc;
				file.write((char *)&pmRA, sizeof(pmRA));
				file.write((char *)&pmDE, sizeof(pmDE));
				file.write((char *)&mag, sizeof(mag));
				file.write((char *)&B_V, sizeof(B_V));
				file.write((char *)&pc, sizeof(pc));
				if (file.bad())
					std::cout << "error writing" << std::endl;

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
	std::cout << "StarManager::saveStarCatalog " << fileName << std::endl;
	std::ofstream file(fileName);
	std::cout.precision(6);

	if (!file.is_open()) {
		std::cout << "Error writing saveStarCatalog" << std::endl;
		return false;
	}

	for(std::vector<HyperCube*>::iterator hc = hyperCubeList.begin(); hc!= hyperCubeList.end(); hc++) {

		file << "H" << " " << (*hc)->getCx() << " " << (*hc)->getCy() << " " << (*hc)->getCz() << " " << (*hc)->getNbrCubes() << std::endl;

		std::vector<Cube*> List = (*hc)->getCubeList();
		for(std::vector<Cube*>::iterator c = List.begin(); c!= List.end(); ++c) {

			file << "C" << " " << (*c)->getCx() << " " <<(*c)->getCy() << " " << (*c)->getCz() << " " << (*c)->getNbStars() << std::endl;

			std::vector<starInfo*> List2 = (*c)->getStarList();
			for(std::vector<starInfo*>::iterator star = List2.begin(); star!= List2.end(); ++star) {

				file << "S" << " " << (*star)->HIP << " " << (*star)->posXYZ[0] << " " << (*star)->posXYZ[1] << " " << (*star)->posXYZ[2] << " " << (*star)->pmRA << " " << (*star)->pmDE << " " << (*star)->mag << " " << (*star)->B_V << " " << (*star)->pc << std::endl;
			}
		}
	}

	file.close();
	return true;
}

//Détermine si un hypercube existe, si oui retourne un pointeur
HyperCube* StarManager::hcExist(int a, int b, int c)
{
	std::vector<HyperCube*> list = getHyperCubeList();
	for(std::vector<HyperCube*>::iterator i = list.begin(); i!= list.end(); ++i) {

		if (((*i)->getCx()== a) && ((*i)->getCy()== b) && ((*i)->getCz()== c) ) {
			return (*i);
		}
	}
	return nullptr;
}


//Trouver dans quel Hypercube se trouve l'étoile
void StarManager::addHcStar(starInfo* star)
{
	float X = star->posXYZ[0];
	float Y = star->posXYZ[1];
	float Z = star->posXYZ[2];

	int hcX = floor((X +HCSIZE/2) / HCSIZE);
	int hcY = floor((Y +HCSIZE/2) / HCSIZE);
	int hcZ = floor((Z +HCSIZE/2) / HCSIZE);

	int hc_centerX = hcX * HCSIZE;
	int hc_centerY = hcY * HCSIZE;
	int hc_centerZ = hcZ * HCSIZE;

	if  ( (abs(hc_centerX-X)>HCSIZE/2) || (abs(hc_centerY-Y)>HCSIZE/2) || (abs(hc_centerZ-Z)>HCSIZE/2) ) {
		std::cout << "hc entree " << X << " " << Y << " " << Z << std::endl;
		std::cout << "hc sortie " << hc_centerX << " " << hc_centerY << " " << hc_centerZ << std::endl;
		//~ sleep(2);
	}

	HyperCube *tmp=nullptr;
	tmp = hcExist(hc_centerX, hc_centerY, hc_centerZ );

	if (tmp == nullptr) { //retourne un pointeur null ni l'hypercube n'existe pas
		tmp = new HyperCube(hc_centerX, hc_centerY, hc_centerZ);
		addHyperCube(tmp); //ajoute un hypercube au StarManager
		tmp->addCubeStar(star);
	} else {
		tmp->addCubeStar(star);
	}
}

// Fonction de lecture du catalogue d'étoile
// Rempli une liste d'étoiles avec celles trouvées dans le catalogue
//
//    I/311 Hipparcos, the New Reduction  (van Leeuwen, 2007)
//    Hipparcos, the new Reduction of the Raw data van Leeuwen F.
//    <Astron. Astrophys. 474, 653 (2007)>
//
//    http://cdsarc.u-strasbg.fr/viz-bin/Cat?I/311
bool StarManager::loadStarRaw(const std::string &catPath)
{
	std::cout << "StarManager::loadStarRaw " << catPath << std::endl;
	std::ifstream file(catPath);
	unsigned int hip;
	float RArad, DErad, Plx, pmRA, pmDE, mag_app, BV;
	unsigned starAccepted=0, starRejected =0;

	//Création de l'hypercube et du cube initial pour contenir le soleil
	Vec3f origin(0.00001, 0.00001, 0.00001);
	starInfo *sun = new starInfo;
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
		cLog::get()->write("Lecture du catalogue initial " + catPath);

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

			starInfo *si = nullptr;
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
}


starInfo* StarManager::createStar(unsigned int hip, float RArad, float DErad, float Plx, float pmRA, float pmDE, float mag_app, float BV)
{
	float x, y, z;
	double parsec;

	// conversion en coordonnées x,y,z
	// on décide de modifier le Plx minimal et de le fixer à 0.2 ce qui fait une étoile au pire à 16000 al
	if (Plx >PLX_MIN) {
		parsec = 1000.0 / Plx; //calcul en parsec
	} else {
		parsec = 1000.0 / (PLX_MIN+0.01* (rand()%10) );
	}

	x = parsec * cos( RArad ) * cos( DErad );
	y = parsec * sin ( DErad );
	z = parsec * sin ( RArad ) * cos( DErad );

	//patch Lionel Ruiz pour mise en conformité avec le repère d'OpenGL
	x = -x;
	y = -y;

	starInfo *si = nullptr;
	si = new starInfo;
	if (si==nullptr)
		return nullptr;

	si->HIP = hip;
	si->posXYZ = Vec3f(x,y,z);
	si->pmRA = pmRA;
	si->pmDE = pmDE;
	si->pc = parsec;
	si->mag = mag_app-5*(log10(parsec)-1);
	si->B_V = (int) ((BV+0.5)/4.*127.);

	if (si->B_V < 0) {
		si->B_V = 0;
		// cout << si->HIP << " " << si ->B_V << endl;
		cLog::get()->write("Star B_V to 0 with hip "+ std::to_string(si->HIP), LOG_TYPE::L_WARNING);
	}
	if (si->B_V >127) {
		si->B_V = 127; 
		// cout << si->HIP << " " << si ->B_V << endl;
		cLog::get()->write("Star B_V to 127 with hip "+ std::to_string(si->HIP), LOG_TYPE::L_WARNING);
	}
	return si;
}

int StarManager::getNbrCubes()
{
	nbrCubes = 0;
	for(int unsigned i = 0; i<hyperCubeList.size(); i++) {
		nbrCubes += hyperCubeList[i]->getNbrCubes();
	}
	return nbrCubes;
}


float StarManager::getMinMagnitude()
{
	//si pas calculé alors calcule le
	if (MinMagnitude==500) {
		for(std::vector<HyperCube*>::iterator i = hyperCubeList.begin(); i!= hyperCubeList.end(); ++i) {
			if( (*i)->getMinMagnitude() <MinMagnitude  )
				MinMagnitude=(*i)->getMinMagnitude();
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
	for(std::vector<HyperCube*>::iterator i = hyperCubeList.begin(); i!= hyperCubeList.end(); ++i) {
		tmp = (*i)->getNbrCubes();
		if (tmp==1) un_cube++;
		if (tmp==tmp_max_cube) max_cube++;
		tmp= tmp/val;
		if (tmp>=NBR_PAS_STATHC) //TODO vraiment utile ?
			tmp = NBR_PAS_STATHC -1;
		statHc[tmp]=statHc[tmp]+1;
	}
	std::cout << std::endl;
	std::cout << "Statistiques sur le nombre de cubes dans les HyperCubes" << std::endl;
	std::cout << std::setw(12) << "Nombre : " << "Hypercubes concernés" << std::endl;
	for(int i=0; i<NBR_PAS_STATHC; i++) {
		std::cout << std::setw(3) << i*8 << " à " << std::setw(3) << (i+1)*8 << " : " << std::setw(12) << statHc[i] << std::endl;
	}
	std::cout << std::endl;
	std::cout << un_cube << " hc avec qu'un cube à gérer" << std::endl;
	std::cout << max_cube << " hc à capacité maximale" << std::endl;
}



void StarManager::MagStarStatistiques()
{
	for(int i=0; i< MAG_PAS; i++)
		statMagStars[i]=0;
	for(std::vector<HyperCube*>::iterator i = hyperCubeList.begin(); i!= hyperCubeList.end(); ++i) {
		std::vector<Cube*> List = (*i)->getCubeList();
		for(std::vector<Cube*>::iterator j = List.begin(); j!= List.end(); ++j) {
			std::vector<starInfo*> List2 = (*j)->getStarList();
			for (std::vector<starInfo*>::iterator k = List2.begin(); k!= List2.end(); ++k) {
				int tmp=(40+(*k)->mag)/5;
				if (tmp<0) tmp=0;
				if (tmp+1>MAG_PAS) tmp=MAG_PAS-1;
				statMagStars[tmp]=statMagStars[tmp]+1;
			}
		}
	}
	std::cout << std::endl;
	std::cout << "Répartition des magnitudes des étoiles" << std::endl;
	std::cout << std::setw(9) << "Magnitude : " << "Nombre d'étoiles" << std::endl;
	for(int i=0; i< MAG_PAS; i++)
		std::cout << std::setw(3) << i*5-40<< " à " << std::setw(3) << i*5-35 << " : " << std::setw(12) << statMagStars[i] << std::endl;
}


bool StarManager::verificationData()
{
	int nbr_cube_max = (HCSIZE/CUBESIZE)*(HCSIZE/CUBESIZE)*(HCSIZE/CUBESIZE);
	//verification des hypercubes
	for(std::vector<HyperCube*>::iterator i = hyperCubeList.begin(); i!= hyperCubeList.end(); ++i) {
		HyperCube *tmp =(*i);
		if (tmp->getCx() %HCSIZE !=0 || tmp->getCy() %HCSIZE !=0 || tmp->getCz() %HCSIZE !=0 ) {
			printf("HyperCube -- coordonnées %i %i %i\n", tmp->getCx(), tmp->getCy(), tmp->getCz());
			return false;
		}

		if (tmp->getNbrCubes()<0 || tmp->getNbrCubes()>nbr_cube_max) {
			printf("HyperCube -%i %i %i- nombre de cubes %i\n", tmp->getCx(), tmp->getCy(), tmp->getCz(), tmp->getNbrCubes());
			return false;
		}

		if (tmp->getNbrCubes()==0) {
			printf("HyperCube -%i %i %i- aucun cube\n", tmp->getCx(), tmp->getCy(), tmp->getCz());
		}

		//verification des cubes
		std::vector<Cube*> List = (*i)->getCubeList();
		for(std::vector<Cube*>::iterator j = List.begin(); j!= List.end(); ++j) {
			Cube *tmp2 = (*j);

			if (tmp2->getCx() %CUBESIZE !=0 || tmp2->getCy() %CUBESIZE !=0 || tmp2->getCz() %CUBESIZE !=0 ) {
				printf("HyperCube -- coordonnées %i %i %i\n", tmp->getCx(), tmp->getCy(), tmp->getCz());
				printf("Cube -- coordonnées %i %i %i\n", tmp2->getCx(), tmp2->getCy(), tmp2->getCz());
				return false;
			}

			if (tmp2->getNbStars()<0 || tmp2->getNbStars()>65000) {
				printf("HyperCube -%i %i %i-\n", tmp->getCx(), tmp->getCy(), tmp->getCz());
				printf("Cube -- coordonnées %i %i %i stars : %i\n", tmp2->getCx(), tmp2->getCy(), tmp2->getCz(), tmp2->getNbStars());
				return false;
			}

			if (tmp2->getNbStars()==0) {
				printf("HyperCube -%i %i %i-\n", tmp->getCx(), tmp->getCy(), tmp->getCz());
				printf("Cube -- coordonnées %i %i %i %i n'a aucune etoile \n", tmp2->getCx(), tmp2->getCy(), tmp2->getCz(), tmp2->getNbStars());
			}
		}
	}
	return true;
}

//TODO la meme en version BINAIRE
bool StarManager::saveAsterismStarsPosition(const std::string &fileNameIn,const std::string &fileNameOut)
{
	std::cout << "StarManager::saveAsterismStarsPosition " << fileNameIn << " " << fileNameOut << std::endl;
	std::ifstream fileIn(fileNameIn, std::ifstream::in);
	std::ofstream fileOut(fileNameOut, std::ifstream::out);
	std::list<int> asterimStars;
	unsigned int HIPName=0;
	starInfo* si= nullptr;

	if (fileIn && fileOut) { // Fails if can't open the file
		std::string line; // variable which will contain each line of the file
		// cout << "Lecture du catalogue "  << fileNameIn << std::endl;
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

starInfo* StarManager::findStar(unsigned int HIPName)
{
	for(std::vector<HyperCube*>::iterator hc = hyperCubeList.begin(); hc!= hyperCubeList.end(); ++hc) {
		std::vector<Cube*> List = (*hc)->getCubeList();
		for(std::vector<Cube*>::iterator c = List.begin(); c!= List.end(); ++c) {
			std::vector<starInfo*> List2 = (*c)->getStarList();
			for(std::vector<starInfo*>::iterator star = List2.begin(); star!= List2.end(); ++star) {
				if ((*star)->HIP == HIPName)
					return (*star);
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
		// cout << "Lecture du catalogue "  << fileNameIn << std::endl;
		cLog::get()->write("StarManager ajout du fichier "+fileName );

		unsigned int hip;
		float ra, de, plx, pmRa, pmDe, mag, bv;
		starInfo * si =nullptr;

		while (getline(fileIn, line)) {
			
			if (line !="") {
				if (line[0]=='#')
					continue;
				
				std::istringstream parseLine(line);
				parseLine >> hip >> ra >> de >> plx >> pmRa >> pmDe >> mag >> bv;

				si = createStar(hip, ra, de, plx, pmRa, pmDe, mag, bv);
				addHcStar(si);
				si = nullptr;
			}
		}
		fileIn.close();
		return true;
	}
	cLog::get()->write("StarManager, loadOtherStar, unable to read file "+ fileName, LOG_TYPE::L_ERROR);

	return false;
}
