#ifndef FILE_WRITER_HPP
#define FILE_WRITER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>

class FileWriter {
public:
	FileWriter(std::string _inHtml);
	~FileWriter();

	/**
	 * Charge le fichier
	 */
	void loadFile(const std::string& fileName);

	/**
	 * Ecrit dans le fichier
	 */
	void writeInFile(const std::string inHtml);

private:
	FILE* wstream;      //Flux d'écriture

	//Fichier de Sortie
	std::string outFile;
};

#endif