/*
 * io_ia.cpp
 * 
 * Copyright 2018 olivier Nivoix <olivier@fixe>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
#include <iostream>
#include <fstream>
#include <array>

#include "vecmath.hpp"



/*
 * 
 * READ !
 * 
 * 
 */

class ReadBinary
{
public:
	ReadBinary(const std::string & fileName);
	~ReadBinary();

	//! initialise la lecture du fichier et active la lecture
	bool start();
	//! ferme le fichier et termine la lecture
	void end();

	//! lit un type et le renvoie
	template<typename T> void read(T& value){
		if (!canUse) 
			return;
	    file.read(reinterpret_cast<char*>(&value), sizeof(T));
		if (!file) {
			error = "Error while reading data";
			canUse = false;
		}
	}

	//! lit une string: le 1° char indique la taille de la string
	void readString(std::string &s);

	std::string getDescription() {
		return description;
	}

	std::string getVersion() {
		return version;
	}

	std::string getError() {
		return error;
	}
	
	explicit operator bool() const {
		 return canUse;
	}

private:
	//! vérifie que le fichier est bien un fichier de SC
	void readSCB();
	//! lit la version du fichier
	void readVersion();
	//! lit la description du fichier
	void readDescription();
	
	bool canUse = false;		// indicateur d'utilisation de la classe
	bool isOpened = false;		// indique si le ficheir a été ouvert par open

	std::ifstream file;
	std::string fileName;
	std::string version;
	std::string description;

	std::string error;			// sommaire gestion d'erreur
};


ReadBinary::ReadBinary(const std::string & _fileName)
{
		fileName = _fileName;
}

ReadBinary::~ReadBinary()
{
	this->end();
}

bool ReadBinary::start()
{
	file.open(fileName, std::ios::binary);
	
	if(!file.is_open()) {
		error = "can't open file " + fileName; 
		canUse = false;
		return false;
	}
	isOpened = true;
	canUse = true;

	readSCB();
	readVersion();
	readDescription();

	return canUse;
}

void ReadBinary::end()
{
	if (isOpened)
		file.close();
}

void ReadBinary::readString(std::string &s)
{
	if (!canUse)
		return;
	unsigned char size=0;
	this->read(size);
	if (!canUse)
		return;
	char* tmp;
	tmp = new char[size];
	//~ std::cout << "size : " << sizeof(tmp) << std::endl;
	file.read(tmp, size);
	if (!file)
		canUse = false;
	tmp[size]='\0';
	s = std::string(tmp);
	delete tmp;
	//~ std::cout << s << " size : " << s.length() << std::endl;
}

void ReadBinary::readSCB()
{
	if (!canUse) return;
	std::array<char,3> b;
	this->read(b);
	if(b[0]=='S' && b[1]=='C' && b[2]=='B')
		return;
	else {
		error = "Not a SC binary file";
		canUse = false;
	}
}

void ReadBinary::readDescription()
{
	if (!canUse) return ;
	char d[32];
	this->read(d);
	description = std::string(d);
	//std::cout << "Description : " << d << std::endl;
}

void ReadBinary::readVersion()
{
	if (!canUse) return ;
	char d[6];
	this->read(d);
	version = std::string(d);
	//std::cout << "Version : " << d << std::endl;
}

/*
 * 
 * WRITE !
 * 
 * 
 */
class WriteBinary
{
public:
	WriteBinary(const std::string & _fileName);
	~WriteBinary();

	//! initialise l'écriture du fichier et active l'écriture dans le fichier
	bool start(const std::string &_description);
	//! ferme le fichier, termine l'écriture des données
	void end();

	//! écrit une string. Le 1° caractère écrit indique la taille de la string
	void writeString(const std::string &s);

	//! écrit un type de base, même les vecteurs
	template<typename T> void write(const T& value) {
		if (!canUse)
			return;
		file.write(reinterpret_cast<const char*>(&value), sizeof(T));
		if (!file) {
			error = "can't write data";
			canUse = false;
		}
	}

	std::string getError() {
		return error;
	}

	explicit operator bool() const {
		 return canUse;
	}

private:
	//! écrit le cartouche de Spacecrafter
	void insertSCB();
	//! écrit la version du fichier
	void insertVersion();
	//! écrit la description du fichier
	void insertDescription(const std::string &_description);

	std::ofstream file;
	std::string fileName;
	std::string error;
	bool isOpened = false;
	bool canUse = false;
};

WriteBinary::WriteBinary(const std::string & _fileName)
{
	fileName = _fileName;
}

WriteBinary::~WriteBinary()
{
	if (isOpened)
		file.close();
}

bool WriteBinary::start(const std::string &_description)
{
	file.open(fileName, std::ios::binary|std::ios::trunc);
	if (!file.is_open()) {
		error = "can't write into file " + fileName;
		return false;
	}
	isOpened = true;
	canUse = true;

	insertSCB();
	insertVersion();
	insertDescription(_description);
	return canUse;
}

void WriteBinary::end()
{
	if (canUse)
		file.close();
}

void WriteBinary::writeString(const std::string &s)
{
	if (!canUse)
		return;
	if (s.length()>255) {
		std::cout << "too length string : " << s<< std::endl;
		return;
	}
	char sLength = (char) s.length();
	this->write(sLength);
	file.write(s.c_str(), s.length());
	if (!file) {
		error = "can write string data";
		canUse = false;
	}
}

void WriteBinary::insertSCB()
{
	if (!canUse)
		return;
	char b[3]={'S','C','B'};
	this->write(b);
}


void WriteBinary::insertVersion()
{
	time_t     now = time(0);
    struct tm  tstruct;
    char buf[6+1]; 
	char buf2[6];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%y%m%d", &tstruct);
	strncpy(buf2, buf, 6);
	//std::cout << "version " << buf << std::endl;
	this->write(buf2);
}

void WriteBinary::insertDescription(const std::string &_description)
{
	char d[32]={'\0'};
	strncpy(d,_description.c_str(),32);
	this->write(d);
}

int main(int argc, char **argv)
{
if (1) {
	std::cout << "Ecriture des données :" << std::endl;

	WriteBinary myFile("olivier.bin");
	myFile.start("Test");

	unsigned long int ni = 5721;
	myFile.write(ni);
	std::cout << "uint ni " << ni << std::endl;

	int nj = -365;
	myFile.write(nj);
	std::cout << "int nj " << nj << std::endl;

	float nf = 0.5f;
	myFile.write(nf);
	std::cout << "float nf " << nf << std::endl;

	unsigned char nc=143;
	myFile.write(nc);
	std::cout << "char nc " << (nc-0) << std::endl;

	char b[3]={'S','O','X'};
	myFile.write(b);
	std::cout << "char tab " << b << std::endl;

	Vec3f m(0.1, 0.5, 0.9);
	std::cout << "Vecteur "<< m[0] << " " << m[1] << " " << m[2] << std::endl;
	myFile.write(m);

	std::string mys = "Il fait beau mais chaud, j'adore le soir me pronener au clair de lune.";
	myFile.writeString( mys);
	std::cout << "string " << mys << " taille "<< mys.length() << std::endl;


	Vec3i n[6];
	n[0] = Vec3i(01, 21, 70);
	n[1] = Vec3i(12, 13, 80);
	n[2] = Vec3i(23, 24, 90);
	n[3] = Vec3i(34, 21, 90);
	n[4] = Vec3i(45, 13, 80);
	n[5] = Vec3i(56, 24, 70);
	std::cout << "Vecteurs "<< n[0] << " " << n[1] << " " << n[2] << " " <<  n[3] << " " << n[4] << " " << n[5]<< std::endl;
	myFile.write(n);

	std::cout << "Erreur ? : "<< myFile.getError() << std::endl;
	myFile.end();
}

	std::cout << std::endl;

if (1) {
	std::cout << "Lecture des données :" << std::endl;

	ReadBinary myFile("olivier.bin");
	myFile.start();


	std::cout  << "Exterieur: version " << myFile.getVersion() << " description " << myFile.getDescription() << std::endl;

	unsigned long int ni;
	myFile.read(ni);
	std::cout << "int ni " << ni << std::endl;

	int nj;
	myFile.read(nj);
	std::cout << "int nj " << nj << std::endl;

	float nf;
	myFile.read(nf);
	std::cout << "float nf " << nf << std::endl;

	unsigned char nc;
	myFile.read(nc);
	std::cout << "char nc " << (nc-0) << std::endl;

	char b[3];
	myFile.read(b);
	std::cout << "char tab " << b << std::endl;

	Vec3f m;
	myFile.read(m);
	std::cout << "Vecteur "<< m[0] << " " << m[1] << " " << m[2] << std::endl;

	std::string mys;
	myFile.readString(mys);
	std::cout << "string " << mys << " taille " << mys.length() << std::endl;

	Vec3i n[6];
	myFile.read(n);
	std::cout << "Vecteurs "<< n[0] << " " << n[1] << " " << n[2] << " " <<  n[3] << " " << n[4] << " " << n[5]<< std::endl;

	std::cout << "Erreur ? : "<< myFile.getError() << std::endl;
	myFile.end();
}

	return 0;
}
