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
//! \file starManager.hpp
//! \brief 3 classes which stock the stars from the catalog
//! \author Robin.L - Olivier NIVOIX
//! \date september 2016

#ifndef STARMANAGER_HPP
#define STARMANAGER_HPP

// defines the length in parsec of the side of a cube
#define CUBESIZE 100

// define the length in parsec of the side of a HyperCube
#define HCSIZE 400

// define related to statistics
#define NBR_PAS_STATHC 8
#define MAG_PAS 12

#include <vector>
#include "tools/vecmath.hpp"
#include <cstdint>
//#include "tools/ia.hpp"

//! \struct StarInfo
//! \brief Stars are stocked in this structure
struct StarInfo {
	unsigned int HIP;	//name of the star
	Vec3f posXYZ;	//position in space in al
	float pmRA;	// RA in mas
	float pmDE;	// DE in mas
	float mag;	//magnitude of the object
	float pc; 	//unit : parsec
	uint8_t B_V;	//color index of the object
	bool show;
};

// GPU need posXYZ, mag, B_V (COMPACT = 4 float, 1 int)
// CPU need HIP


//! \class Cube
//! \brief la classe cube va contenir les etoiles
class Cube  {
public:
	//! \fn Cube(int size, int x, int y, int z, int cubeName)
	//! \brief Cube's constructor
	//! \param size is the cube size
	//! \return create a cube
	Cube(int x, int y, int z);

	//! \brief Cube's destructor
	~Cube();

	//! \return return the size of the cube
	size_t getSize() {
		return size;
	}

	//! \brief getter sur la position du cube
	int getCx() {
		return c_x;
	}
	int getCy() {
		return c_y;
	}
	int getCz() {
		return c_z;
	}

	//! \fn void addStar(StarInfo *si)
	//! \brief add a star to the starList
	//! \param *si pointer on a StarInfo type
	void addStar(StarInfo &&si);

	//! \fn return the number of stars
	int getNbStars() {
		return starList.size();
	}

	inline std::vector<StarInfo>::iterator begin() {
		return starList.begin();
	}

	inline std::vector<StarInfo>::iterator end() {
		return starList.end();
	}

	//! \brief renvoi le nombre total de cube dans la structure
	static unsigned int getTotalCube() {
		return NbTotalCube;
	}

	//! \brief renvoie la magnitude minimale de ce cube
	float getMinMagnitude() {
		return MinMagnitude;
	}

protected:
	int size;
	int c_x, c_y, c_z;
	std::vector<StarInfo> starList;
	float MinMagnitude;
	static unsigned int NbTotalCube;
};


/*!
 * \class HyperCube
 * \brief Classe stockant un ensemble de cubes inclus dans son propre volume
 * et permettant sa gestion
 */
class HyperCube  {
public:
	//! \fn HyperCube(int  x, int y, int z)
	//! \brief constructor of hypercube
	//! \param hcSize hyperCube's size
	HyperCube(int x, int y, int z);

	//! \brief hypercube's destructor
	~HyperCube();

	//! \return return the hyperCube size
	int getSize() {
		return hcSize;
	}

	inline std::vector<Cube>::iterator begin() {
		return cubeList.begin();
	}

	inline std::vector<Cube>::iterator end() {
		return cubeList.end();
	}

	//! \return return the cubes which are currently in the hypercube
	int getNbrCubes() {
		return cubeList.size();
	}

	//! \brief getter sur la position du cube
	int getCx() {
		return c_x;
	}
	int getCy() {
		return c_y;
	}
	int getCz() {
		return c_z;
	}

	//! \brief Renvoie le cube en coordonnee (a,b,c), cree un cube si besoin
	//! \return Cube aux coordonnees demandees
	Cube &getCube(int a, int b, int c);

	void addCubeStar(StarInfo &&star);

	//! \brief renvoie la magnitude minimale de l'etoile inclue dans l'HC
	float getMinMagnitude();

	//! \brief renvoie le nombre total de cube dans l'HC
	unsigned int getTotalHyperCube() {
		return NbTotalHyperCube;
	}

	//! renvoie le nombre d'etoiles inclues dans l'HC
	unsigned int getNbrStars();

protected:
	int hcSize;
	int c_x;
	int c_y;
	int c_z;
	std::vector<Cube> cubeList;
	int min,max;
	float MinMagnitude;
	static unsigned int NbTotalHyperCube;
};

/*! \class StarManager
 * \brief Classe stockant tous les HC necessaire a la representation des etoiles
 * dans le logiciel
 *
 * \details Pour stocker et utiliser toutes les etoiles, StarManager decoupe l'espace
 * en plusieurs HyperCubes HC contenant eux meme des Cubes C qui eux contiennent
 * les etoiles.
 *
 * Aussi l'espace est partitionne en gros cubes contenant des cubes afin de pouvoir
 * faire des optimisations par exemple ...
 *
 */
class StarManager {
public:
	//! \brief starManager constructor
	StarManager();

	//! \brief StarManager destructor
	~StarManager();

	//! \return return the number of hypercube
	int getNbrHyperCubes() {
		return hyperCubeList.size();
	}

	//! recupere le nombre de cubes dans les hypercubes
	//! \return return the total number of cubes
	int getNbrCubes();

	//! \return return the hypercube list which is in the starManager
	//! \warning The existence of such function break the OOP paradigm
	std::vector<HyperCube> &getHyperCubeList() {
		return hyperCubeList;
	}

	//! \brief read the catalogue created before by the programm
	//! \return vrai si toutes les lignes ont ete inserees dans le programme.
	bool loadStarCatalog(const std::string &fileName);

	//! \brief read the binary catalogue created before by the programm
	//! \return vrai si toutes les lignes ont ete inserees dans le programme.
	bool loadStarBinCatalog(const std::string &fileName);

	//! \brief lit un autre catalogue d'etoiles dans le manager
	//! \return vrai si toutes les lignes ont ete inserees dans le programme.
	bool loadOtherStar(const std::string & fileName);

	//! \brief lecture du catalogue initial et creation des objets pour le contenir
	bool loadStarRaw(const std::string &starPath);

	//! \brief used to save the stars/cubes/hypercube in the structure
	bool saveStarCatalog(const std::string &fileName);

	//! \brief used to save in binary mode the stars/cubes/hypercube in the structure
	bool saveStarBinCatalog(const std::string &fileName);

	//! \brief Renvoie le hypercube aux coordonnees demandees, en le creant s'il n'existe pas.
	HyperCube &getHC(int a, int b, int c);

	//! \brief Ajoute une etoile dans starManager
	void addHcStar(StarInfo &&star);

	//! \brief renvoie la plus grande magnitude absolue de toutes les etoiles
	float getMinMagnitude();

	//! \brief renvoie des statistiques concernant le remplissage des HyperCubes
	void HyperCubeStatistiques();

	//! \brief renvoie des statistiques concernant les magnitudes des etoiles
	void MagStarStatistiques();

	//! \brief renvoie le nombre d'etoiles dans toute la structure
	unsigned int getNbrStars();

	//! \brief fonction de verification des donnees dans la structure
	bool verificationData();

	//! \brief sauvegarde les coordonnees des etoiles issues du fichier filenameIn
	//! dans le fichier filenameOut.
	bool saveAsterismStarsPosition(const std::string &fileNameIn,const std::string &fileNameOut);

	//! \brief renvoie les caracteristiques de l'etoile identifiee par HIPName
	StarInfo *findStar(unsigned int HIPName);

protected:
	std::vector<HyperCube> hyperCubeList;
	int nbrCubes = 0;
	float MinMagnitude = 500;
	int statHc[NBR_PAS_STATHC] {0};
	unsigned int statMagStars[MAG_PAS];
};

#endif
