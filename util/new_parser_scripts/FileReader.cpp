#include "FileReader.hpp"
#include "DefineFile.hpp"

FileReader::FileReader() {}
FileReader::~FileReader() {}

std::string FileReader::init(std::string source, std::string destination) {
	//Fichier source
	rstream = fopen(source.c_str(), "r");
	if(rstream == NULL) {
		perror("Erreur d'ouverture du fichier source en lecture ");
		exit(FOPEN_SOURCE_ERROR);	
	}
	
	//Fichier html de destination
	wstream = fopen(destination.c_str(), "w");
	if(rstream == NULL) {
		fclose(rstream);
		perror("Erreur d'ouverture du fichier html de destination en écriture ");
		exit(FOPEN_DESTINATION_ERROR);
	}
	
	//Buffer contenant la ligne courante
	std::string line;
	//line = malloc(sizeof(*line) * BUFFER_SIZE);
	if(line == "") {
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

std::string FileReader::nextline() {
	std::string line = realline;
	char *cstr = new char[line.length() + 1];
	
	if(fgets(strcpy(cstr, line.c_str()), BUFFER_SIZE, rstream) == NULL) {
		perror("Erreur ou fin du fichier ");
		end(FGETS_ERROR);
	}
	line = noblank(line);
	return line;
}

std::string FileReader::noblank(std::string line) {
	while(line[0] == SPACE || line[0] == TAB) line;
	return line;
}

std::string FileReader::noblanknorline(std::string line) {
	line = noblank(line);
	while (line[0] == NEWLINE) {
		line = nextline();
		line = noblank(line);
	}
	return line;
}