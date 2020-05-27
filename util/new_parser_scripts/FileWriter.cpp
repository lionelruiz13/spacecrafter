#include "FileWriter.hpp"
#include "FileReader.hpp"
#include "TextToHtml.hpp"
#include "DefineFile.hpp"

FileWriter::FileWriter() {
    Reader = new FileReader();
    text = new TextToHtml();
}

FileWriter::~FileWriter() {}

void FileWriter::writetext(char* text) {
	if(fputs(text, wstream) == EOF) {
		perror("Erreur d'écriture ");
		Reader->end(FPUTS_ERROR);
	}
}

void FileWriter::writenchar(char* line, int  n) {
	if(!fwrite(line, n, 1, wstream)) {
		perror("Erreur d'écriture ");
		Reader->end(FWRITE_ERROR);
	}
}

void FileWriter::writenewline() {
	writetext(HTML_NEWLINE);
}

void FileWriter::writetextnchar(char* begin, char* line, int  n, char* end) {
	writetext(begin);
	writenchar(line, n);
	writetext(end);
}

char* FileWriter::writehtml(char* line) {
	line = Reader->noblanknorline(line);
	int  i=0;
	int  lastchar = 0;
	while(line[i] != SEPARATOR_1 && line[i] != SEPARATOR_2) {
		if(line[i] == NEWLINE) {
			if (lastchar) writenchar(line, lastchar+1);
			line = Reader->nextline();
			if(line[0] != SEPARATOR_1 && line[0] != SEPARATOR_2 && !(text->checktag(line))) writenewline();
			else return line;
			i=0;//Réinitialise l'itérateur comme c'est une nouvelle ligne
			lastchar=0;//Idem pour le dernier caractère utile
			
		}
		if(line[i] != SPACE && line[i] != TAB) lastchar = i;
		i++;
	}
	if (lastchar) writenchar(line, lastchar+1);
	return line+i;
}

char* FileWriter::writetexthtml(char* begin, char* line, char* end) {
	writetext(begin);
	line = writehtml(line);
	writetext(end);
	return line;
}

char* FileWriter::writetextdescpart(char* begin, char* line, char* end) {
	writetext(begin);
	line = text->descpart(line);
	writetext(end);
	return line;
}

int FileWriter::argclose(int argopen) {
	if(argopen) writetext(AFTER_ARGUMENT_LIST);
	return 0;
}