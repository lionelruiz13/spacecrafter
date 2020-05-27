#include "FileReader.hpp"
#include "DefineFile.hpp"

FileReader::FileReader() {}
FileReader::~FileReader() {}

char* FileReader::init(char* source, char* destination) {
	//Fichier source
	rstream = fopen(source, "r");
	if(rstream == NULL) {
		perror("Erreur d'ouverture du fichier source en lecture ");
		exit(FOPEN_SOURCE_ERROR);	
	}
	
	//Fichier html de destination
	wstream = fopen(destination, "w");
	if(rstream == NULL) {
		fclose(rstream);
		perror("Erreur d'ouverture du fichier html de destination en écriture ");
		exit(FOPEN_DESTINATION_ERROR);
	}
	
	//Buffer contenant la ligne courante
	char* line;
	//line = malloc(sizeof(*line) * BUFFER_SIZE);
	if(line == NULL) {
		perror("Erreur d'allocation du buffer ");
		end(MALLOC_ERROR);
	 }
	realline = line;
	return line;
}

void FileReader::end(int code) {
	fclose(rstream);
	fclose(wstream);
	exit(code);
}

char* FileReader::nextline() {
	char* line = realline;
	if(fgets(line, BUFFER_SIZE, rstream) == NULL) {
		perror("Erreur ou fin du fichier ");
		end(FGETS_ERROR);
	}
	line = noblank(line);
	return line;
}

char* FileReader::noblank(char* line) {
	while(line[0] == SPACE || line[0] == TAB) line++;
	return line;
}

char* FileReader::noblanknorline(char* line) {
	line = noblank(line);
	while (line[0] == NEWLINE) {
		line = nextline();
		line = noblank(line);
	}
	return line;
}