/*
Programme de copie de partition psychologiquement agréable
Utilité : ce programme permet de copier des partitions de même taille et affiche une barre de défilement ainsi qu'une estimation du temps restant
Usage : à inclure dans un système
Auteur : Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour association-sirius.org
Mise à jour le 02/06/2016
*/
#ifndef partitioncopy_HPP
#define partitioncopy_HPP

#include <fcntl.h>
#include <linux/fs.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <ncurses.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <blkid/blkid.h>

#define EMPTY_LIST 1
#define WRONG_CHOICE 2
#define ABORT 3
#define INPUT_BLOCK_SIZE_NULL 4
#define INPUT_SIZE_NULL 5
#define OUTPUT_BLOCK_SIZE_NULL 6
#define OUTPUT_SIZE_NULL 7
#define DIFFERENT_SIZE 8
#define DIFFERENT_UUID 9
#define READ_ERROR 10
#define WRITE_ERROR 11

class PartitionCopy
{
	private:
	
	unsigned int line;//Position verticalle actuelle
	std::vector<std::string> partitions;//Liste des partitions
	char* out;//Buffer utilisé pour retourner des chaînes de caractères
	
	void error(int code);
	
	void printBar(unsigned long progress);
	void printPercent(unsigned long progress);
	void printSpeed(unsigned long bytes, unsigned long time);
	void printRemainingTime(unsigned long progress, unsigned long elapsed);
	
	const char* humanReadable(unsigned long bytes);
	
	
	const char* getPartitionInfo(const char* partition, const char* info);
	const char* getPartitionInfos(const char* partition);
	long unsigned int getPartitionSize(const char* partition);
	void fillPartitionList();
	int filterPartitionList(unsigned long minBlockCount);
	int filterSizePartitionList(unsigned int size);
	void filterTypePartitionList(const char* type);
	void filterNamePartitionList(const char* name);
	int printPartitonList();
	const char* chosePartition();
	void initNcurses();
	void renamePartition(char* partition, char* type, char* label);
	


	
	
	
	public:
		int run(int argc, char **argv);
};


#endif
