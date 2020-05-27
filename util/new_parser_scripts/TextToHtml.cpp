#include "TextToHtml.hpp"
#include "FileWriter.hpp"
#include "FileReader.hpp"
#include "DefineFile.hpp"

TextToHtml::TextToHtml() {
    Reader = new FileReader();
    Writer = new FileWriter();
}

TextToHtml::~TextToHtml() {}

int TextToHtml::checktag(char* line) {
	if(line[0] == 'N' && line[1] == 'A' && line[2] == 'M' && line[3] == 'E') return NAME;
	else if(line[0] == 'A' && line[1] == 'R' && line[2] == 'G' && line[3] == 'U' && line[4] == 'M' && line[5] == 'E' && line[6] == 'N' && line[7] == 'T') return ARGUMENT;
	else if(line[0] == 'E' && line[1] == 'X' && line[2] == 'E' && line[3] == 'M' && line[4] == 'P' && line[5] == 'L' && line[6] == 'E') return EXEMPLE;
	else if(line[0] == 'I' && line[1] == 'M' && line[2] == 'G') return IMG;
	else if(line[0] == 'E' && line[1] == 'N' && line[2] == 'D') return END;
	else return 0;
}

char* TextToHtml::name(char* line, char** nametab, int  namenum){
	line = Reader->noblanknorline(line);
	int  i=0;
	int  lastchar = 0;
	while(line[i] != SEPARATOR_1 && line[i] != NEWLINE) {
		if(line[i] != SPACE && line[i] != TAB) lastchar = i;
		i++;
	}
	if (lastchar) {
		int  size = lastchar+1;
		Writer->writetext(BEFORE_NAME);
		Writer->writenchar(line, size);
		Writer->writetext(AFTER_NAME_ID);
		Writer->writenchar(line, size);
		Writer->writetext(AFTER_NAME);
		//nametab[namenum] = malloc(sizeof(*nametab[namenum]) * size+1);
		if(nametab[namenum] == NULL) {
			perror("Erreur d'allocation du nom de la commande dans le tableau ");
			Reader->end(MALLOC_ERROR);
		}
		int  j;
		for(j=0; j < size; j++) nametab[namenum][j] = line[j];
		nametab[namenum][size] = '\0';
	}
	else {
		printf("La commande doit avoir un nom !\n");
		Reader->end(EMPTY_NAME_ERROR);
	}
	line += i;
	line = Reader->noblanknorline(line);
	line = descpart(line);
	Writer->writetext(AFTER_NAME_HEAD);
	return line;
}

char* TextToHtml::descpart(char* line){
	if(line[0] == SEPARATOR_1) {
		line = Writer->writetexthtml(BEFORE_DESC, line+1, AFTER_DESC);
		if(line[0] == SEPARATOR_1) line = Writer->writetexthtml(BEFORE_PART, line+1, AFTER_PART);
	}
	return line;
}

char* TextToHtml::argument(char* line){
	line = Writer->writetexthtml(BEFORE_VARIABLE, line+1, AFTER_VARIABLE);
	if(line[0] == SEPARATOR_1) {
		line = Writer->writetexthtml(BEFORE_TYPE, line+1, AFTER_TYPE);
		line = descpart(line);
	}
	else Writer->writetext(AFTER_ARGUMENT_WITHOUT_TYPE);
	if(line[0] == SEPARATOR_2) {
		Writer->writetext(BEFORE_VALEUR_LIST);
		do {
			line = Writer->writetexthtml(BEFORE_VALEUR, line+1, AFTER_VALEUR);
			line = descpart(line);
			Writer->writetext(AFTER_VALEUR_ELEMENT);
		} while(line[0] == SEPARATOR_2);
		Writer->writetext(AFTER_VALEUR_LIST);
	}
	return line;
}

char* TextToHtml::example(char* line){
	line = Reader->noblanknorline(line);
	int  i=0;
	int  lastchar=0;
	Writer->writetext(BEFORE_EXEMPLE);
	while(line[i] != SEPARATOR_1 && line[i+1] != SEPARATOR_1) {
		if(line[i] == NEWLINE) {
			Writer->writenchar(line, lastchar+1);
			line = Reader->nextline();
			if(line[0] == SEPARATOR_1 && line[1] == SEPARATOR_1) {
				Writer->writetext(AFTER_EXEMPLE);
				return 	line = line+2;
			}
			Writer->writetext("\n");
			i=0;
			lastchar=0;
			
		}
		if(line[i] != SPACE && line[i] != TAB) lastchar = i;
		i++;
	}
	Writer->writenchar(line, lastchar+1);
	Writer->writetext(AFTER_EXEMPLE);
	return 	line = line+i+3;
}

char* TextToHtml::img(char* line) {
	line = Reader->noblanknorline(line);
	int  i=0;
	int  lastchar=0;
	while(line[i] != NEWLINE) {
		if(line[i] != SPACE && line[i] != TAB) lastchar = i;
		i++;
	}
	Writer->writetextnchar(BEFORE_IMG, line, lastchar+1, AFTER_IMG);
	return Reader->nextline();
}