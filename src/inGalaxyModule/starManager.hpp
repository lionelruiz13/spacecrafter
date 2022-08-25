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
//#include "tools/ia.hpp"

//! \struct starInfo
//! \brief Stars are stocked in this structure
struct starInfo {
	unsigned int HIP;	//name of the star
	Vec3f posXYZ;	//position in space in al
	float pmRA;	// RA in mas
	float pmDE;	// DE in mas
	float mag;	//magnitude of the object
	int B_V;	//color index of the object
	float pc; 	//unit : parsec
};




//! \class Cube
//! \brief la classe cube va contenir les étoiles
class Cube  {
public:
	Cube() {}

	//! \fn Cube(int size, int x, int y, int z, int cubeName)
	//! \brief Cube's constructor
	//! \param size is the cube size
	//! \return create a cube
	Cube(int x, int y, int z);

	//! \brief Cube's destructor
	~Cube();

	//! \return return the size of the cube
	int getSize() {
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

	//! \fn void addStar(starInfo *si)
	//! \brief add a star to the starList
	//! \param *si pointer on a starInfo type
	void addStar(starInfo *si);

	//! \fn return the number of stars
	int getNbStars() {
		return starList.size();
	}

	//! \brief getter sur la lsite des étoiles du cube
	std::vector<starInfo*> getStarList() {
		return starList;
	}

	//! \brief renvoi le nombre total de cube dans la structure
	unsigned int getTotalCube() {
		return NbTotalCube;
	}

	//! \brief renvoie la magnitude minimale de ce cube
	float getMinMagnitude() {
		return MinMagnitude;
	}

protected:
	int size;
	int c_x, c_y, c_z;
	std::vector<starInfo*> starList;
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

	//! \return return the cube list for an hyperCube
	std::vector<Cube*> getCubeList() {
		return cubeList;
	}

	//! \brief add a cube to the HyperCube
	void addCube(Cube *c);

	//! \return return the cubes which are currently in the hypercube
	int getNbrCubes() {
		return nbrCubes;
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

	//! \brief Vérifie qu'un cube existe en coordonnée (a,b,c)
	//! \return pointeur sur le cube s'il existe nullptr sinon
	Cube* cubeExist(int a, int b, int c);

	//! \brief ajoute une étoile dans l'Hypercube, crée un cube si besoin
	void addCubeStar(starInfo* star);

	//! \brief renvoie la magnitude minimale de l'étoile inclue dans l'HC
	float getMinMagnitude();

	//! \brief renvoie le nombre total de cube dans l'HC
	unsigned int getTotalHyperCube() {
		return NbTotalHyperCube;
	}

	//! renvoie le nombre d'étoiles inclues dans l'HC
	unsigned int getNbrStars();

protected:
	int nbrCubes;
	int hcSize;
	int c_x;
	int c_y;
	int c_z;
	std::vector<Cube*> cubeList;
	int min,max;
	float MinMagnitude;
	static unsigned int NbTotalHyperCube;
};

/*! \class StarManager
 * \brief Classe stockant tous les HC nécessaire à la représentation des étoiles
 * dans le logiciel
 *
 * \details Pour stocker et utiliser toutes les étoiles, StarManager découpe l'espace
 * en plusieurs HyperCubes HC contenant eux même des Cubes C qui eux contiennent
 * les étoiles.
 *
 * Aussi l'espace est partitionné en gros cubes contenant des cubes afin de pouvoir
 * faire des optimisations par exemple ...
 *
 */
class StarManager {
public:
	//! \brief starManager constructor
	StarManager();

	//! \brief StarManager destructor
	~StarManager();

	//! \fn void addHyperCubeList(HyperCube *hc)
	//! \brief add an hypercube into the list
	//! \param *hc pointer of hypercube type
	void addHyperCube(HyperCube *hc);

	//! \return return the number of hypercube
	int getNbrHyperCubes() {
		return nbrHyperCubes;
	}

	//! récupère le nombre de cubes dans les hypercubes
	//! \return return the total number of cubes
	int getNbrCubes();

	//! \return return the hypercube list which is in the starManager
	std::vector<HyperCube*> getHyperCubeList() {
		return hyperCubeList;
	}

	//! \brief read the catalogue created before by the programm
	//! \return vrai si toutes les lignes ont été insérées dans le programme.
	bool loadStarCatalog(const std::string &fileName);

	//! \brief read the binary catalogue created before by the programm
	//! \return vrai si toutes les lignes ont été insérées dans le programme.
	bool loadStarBinCatalog(const std::string &fileName);

	//! \brief lit un autre catalogue d'étoiles dans le manager
	//! \return vrai si toutes les lignes ont été insérées dans le programme.
	bool loadOtherStar(const std::string & fileName);

	//! \brief lecture du catalogue initial et création des objets pour le contenir
	bool loadStarRaw(const std::string &starPath);

	//! \brief used to save the stars/cubes/hypercube in the structure
	bool saveStarCatalog(const std::string &fileName);

	//! \brief used to save in binary mode the stars/cubes/hypercube in the structure
	bool saveStarBinCatalog(const std::string &fileName);

	//! \brief Détermine si un hypercube existe, si oui retourne un pointeur sur ce dernier
	HyperCube* hcExist(int a, int b, int c);

	//! \brief Ajoute une étoile dans starManager
	void addHcStar(starInfo* star);

	//! \brief renvoie la plus grande magnitude absolue de toutes les étoiles
	float getMinMagnitude();

	//! \brief renvoie des statistiques concernant le remplissage des HyperCubes
	void HyperCubeStatistiques();

	//! \brief renvoie des statistiques concernant les magnitudes des étoiles
	void MagStarStatistiques();

	//! \brief renvoie le nombre d'étoiles dans toute la structure
	unsigned int getNbrStars();

	//! \brief fonction de vérification des données dans la structure
	bool verificationData();

	//! \brief sauvegarde les coordonnées des étoiles issues du fichier filenameIn
	//! dans le fichier filenameOut.
	bool saveAsterismStarsPosition(const std::string &fileNameIn,const std::string &fileNameOut);

	//! \brief renvoie les caractéristiques de l'étoile identifiée par HIPName
	starInfo* findStar(unsigned int HIPName);

protected:
	std::vector<HyperCube*> hyperCubeList;
	int nbrCubes;
	int nbrHyperCubes;
	float MinMagnitude;
	int statHc[NBR_PAS_STATHC];
	unsigned int statMagStars[MAG_PAS];
	starInfo* createStar(unsigned int hip, float ra, float de, float plx, float pmRa, float pmDe, float mag, float bv);
};

#endif
