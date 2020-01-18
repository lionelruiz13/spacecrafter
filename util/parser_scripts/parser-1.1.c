/*
Programme de génération de fichier HTML5
Utilité: ce programme permet de générer des fichiers HTML5 de documentation à base de fichiers spécifiques
Usage: script fichier-source fichier-html-de-destination
Auteur: Aurélien Schwab <aurelien.schwab+dev@gmail.com> pour association-sirius.org
Mise à jour le: 03/09/2017
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define BUFFER_SIZE 8192
#define NAME_TAB_SIZE 512

//Définition du html

#define HTML_NEWLINE "\n<br/>"

#define BEFORE_STYLE "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=\"UTF-8\">\n<title>Documentation</title>\n"

#define AFTER_STYLE "</head>\n<body>\n<header>\n<h1>Documentation du logiciel</h1>\n</header>\n<img src=\"img/logo.png\" alt=\"Logo\" class=\"logo\">\n<section class=\"commande\">\n"

#define BEFORE_NAME "<article id=\""
#define AFTER_NAME_ID "\">\n<header>\n<h2><code>"
#define AFTER_NAME "</code></h2>\n"
#define AFTER_NAME_HEAD "</header>\n"

#define BEFORE_DESC "<p class=\"description\">"
#define AFTER_DESC "</p>\n"

#define BEFORE_PART "<p class=\"particularite\">"
#define AFTER_PART "</p>\n"

#define BEFORE_ARGUMENT_LIST "<section class=\"listearguments\">\n<h3>Arguments</h3>\n<ol>\n"
#define BEFORE_ARGUMENT_VARIABLE "<li>\n<h4><code class=\"argument\">"
#define AFTER_ARGUMENT_VARIABLE "</code>"
#define BEFORE_ARGUMENT_TYPE " : <code class=\"argumenttype\">"
#define AFTER_ARGUMENT_TYPE "</code></h4>\n"
#define AFTER_ARGUMENT_WITHOUT_TYPE "</h4>\n"
#define AFTER_ARGUMENT_LIST "</ol>\n</section>\n"

#define BEFORE_VALEUR_LIST "<section class=\"listevaleurs\">\n<ul>\n"
#define BEFORE_VALEUR "<li>\n<code class=\"valeur\">"
#define AFTER_VALEUR "</code>\n"
#define AFTER_VALEUR_ELEMENT "</li>\n"
#define AFTER_VALEUR_LIST "</ul>\n</section>\n"

#define BEFORE_EXEMPLE "<section class=\"exemple\">\n<h2>Exemple</h2>\n<pre>"
#define AFTER_EXEMPLE "</pre>\n</section>\n"

#define BEFORE_IMG "<img src=\"img/"
#define AFTER_IMG "\" alt=\"| Image |\">\n"

#define AFTER_BLOCK "</article>\n<a href=\"#Menu\" class=\"retour\">Retour à l'index</a>\n"

#define BEFORE_MENU "</section>\n<aside id=\"Menu\" class=\"menu\">\n<h3>Index</h3>\n<ol>\n"
#define BEFORE_MENU_NAME "<li><a href=\"#"
#define AFTER_MENU_LINK "\"><code>"
#define AFTER_MENU_NAME "</code></a></li>\n"
#define AFTER_MENU "</ol>\n</aside>\n"

#define FOOT "</body>\n</html>"

//Définition des séparateurs
#define SEPARATOR_1 '@'//Doublé pour la balise de fin de l'exemple
#define SEPARATOR_2 '$'

//Définition des caractères
#define SPACE ' '
#define TAB '\t'
#define NEWLINE '\n'

//Constantes internes
#define NAME 1
#define ARGUMENT 2
#define EXEMPLE 3
#define IMG 4
#define END 5

#define SIZE_NAME 4
#define SIZE_ARGUMENT 8
#define SIZE_EXEMPLE 7
#define SIZE_IMG 3

#define USAGE_ERROR 1
#define EMPTY_NAME_ERROR 2
#define MALLOC_ERROR 3
#define FOPEN_SOURCE_ERROR 4
#define FOPEN_DESTINATION_ERROR 5
#define FOPEN_STYLE_ERROR 6
#define FGETS_ERROR 7
#define FPUTS_ERROR 8
#define FWRITE_ERROR 9

FILE* rstream;//Flux de lecture
FILE* wstream;//Flux d'écriture
char* realline;//Constante contenant la position initiale du pointeur du buffer

//Traitement des balises
int checktag(char* line) ;//Renvoi l'entier associé à chaque balise
char* name(char* line,char** nametab, int  namenum);//Traite la balise NAME
char* descpart(char* line);//Traie la partie description et particularité des balises
char* argument(char* line);//Traite la balise ARGUMENT
char* example(char* line);//Traite la balise EXEMPLE qui est un cas particulier car on peut tout mettre dedans sauf @@ qui est la balise de fermeture
char* img(char* line);//Traite la balise IMG qui est aussi un cas particulier car 

//Déplacement du pointeur
char* nextline();//Place le poineur à la ligne suivante du fichier
char* noblank(char* line);//Ignore tous les espaces et tabulations
char* noblanknorline(char* line);//Ignore tous les espaces et les tabulations ainsi que les lignes qui ne contiennent que ça

//Ecriture du texte
void writetext(char* text);//Ecrit du texte dans le fichier
void writenchar(char* line, int  n);//Ecrite les n premier caractères
void writenewline() ;//Saute une ligne en html
void writetextnchar(char* begin, char* line, int  n, char* end);//Permet d'écrire autour de writenchar(line, n)
char* writehtml(char* line);//Ecrit dans le html et s'arrête à un des deux séparateur ou une balise après un saut de ligne (n'écrit pas les espaces après la balise et avant la fin)
char* writetexthtml(char* begin, char* line, char* end);//Permet d'écrire autour de writehtml(line)
char* writetextdescpart(char* begin, char* line, char* end);//Permet d'écrire autour de writetextdescpart(line)
int argclose(int argopen);//Ferme une liste d'arguments ouverte

//Début et fin du programme
char* init(char* source, char* destination);//Ouvre les fichiers et alloue le buffer de la ligne courante
void end(int code);//Termine le programme en fermant les fichiers


int main(int argc, char *argv[]) {
	char* line;
	switch(argc) {
		case 3 : //2 arguments (pas de fichier de style)
			line = init(argv[1], argv[2]);
			writetext(BEFORE_STYLE);
			break;
		case 4 : //3 arguements (fichier de style)
			line = init(argv[1], argv[3]);
			//Fichier de style
			FILE* rstylestream = fopen(argv[2], "r"); //Ouverture du fichier de style en lecture
			if(rstream == NULL) {
				perror("Erreur d'ouverture du fichier style en lecture ");
				exit(FOPEN_STYLE_ERROR);	
			}
			writetext(BEFORE_STYLE);
			writetext("<style>\n");
			while(fgets(line, BUFFER_SIZE, rstylestream) != NULL) writetext(line); //Intégration du fichier de style dans la page
			writetext("\n</style>\n");
			fclose(rstylestream); //Fermeture du fichier de style
			break;
			
		default : //Mauvaise utilisation de la commande
			printf("usage : %s fichier-source [fichier-de-style] fichier-html-de-destination\n", argv[0]);
			exit(USAGE_ERROR);
			break;
	
	
	}
	char** nametab;//Pointeur sur le tableau de noms de commandes
	writetext(AFTER_STYLE);
	line = nextline();//Passe à la première ligne
	int  blockopen = 0;//Témoin de block ouvert
	int argopen = 0;//Témoin de liste d'arguments ouverte
	while(1) {
		switch(checktag(line)) {
			case NAME :
				argopen = argclose(argopen);
				if(blockopen) writetext(AFTER_BLOCK);
				else {
					nametab = malloc(sizeof(*nametab) * NAME_TAB_SIZE);
					if(line == NULL) {
						perror("Erreur d'allocation du tableau de noms : ");
						end(MALLOC_ERROR);
					 }
				}
				line = name(line+SIZE_NAME, nametab, blockopen);
				blockopen++;
				break;
				
			case ARGUMENT :
				if(!argopen) {
					writetext(BEFORE_ARGUMENT_LIST);
					argopen++;
				}
				line = argument(line+SIZE_ARGUMENT);
				break;
				
			case EXEMPLE :
				argopen = argclose(argopen);
				line = example(line+SIZE_EXEMPLE);
				break;
				
			case IMG :
				line = img(line+SIZE_IMG);
				break;
				
			case END :
				argclose(argopen);
				if(blockopen) {
					writetext(AFTER_BLOCK);
					writetext(BEFORE_MENU);
					int  i;
					for(i=0; i < blockopen; i++) {
						writetext(BEFORE_MENU_NAME);
						writetext(nametab[i]);
						writetext(AFTER_MENU_LINK);
						writetext(nametab[i]);
						writetext(AFTER_MENU_NAME);
					}
					writetext(AFTER_MENU);
				}
				writetext(FOOT);
				end(0);
				
			default : line = nextline();
		}
	}
}

int checktag(char* line) {
	if(line == strstr(line, "NAME")) return NAME;
	else if(line == strstr(line, "ARGUMENT")) return ARGUMENT;
	else if(line == strstr(line, "EXEMPLE")) return EXEMPLE;
	else if(line == strstr(line, "IMG")) return IMG;
	else if(line == strstr(line, "END")) return END;
	else return 0;
}

char* name(char* line, char** nametab, int  namenum){
	line = noblanknorline(line);
	int  i=0;
	int  lastchar = 0;
	while(line[i] != SEPARATOR_1 && line[i] != NEWLINE) { //Tant qu'on est pas à la fin de la ligne et qu'on a pas atteint un caractère de début de balise on ingore tout
		if(line[i] != SPACE && line[i] != TAB) lastchar = i;
		i++;
	}
	if (lastchar) { //Si la ligne contient quelque chose
		int  size = lastchar+1;
		writetext(BEFORE_NAME);
		writenchar(line, size);
		writetext(AFTER_NAME_ID);
		writenchar(line, size);
		writetext(AFTER_NAME);
		nametab[namenum] = malloc(sizeof(*nametab[namenum]) * size+1);
		if(nametab[namenum] == NULL) {
			perror("Erreur d'allocation du nom de la commande dans le tableau ");
			end(MALLOC_ERROR);
		}
		int  j;
		for(j=0; j < size; j++) nametab[namenum][j] = line[j]; //Ajout du nom dans la liste
		nametab[namenum][size] = '\0';
	}
	else {
		printf("La commande doit avoir un nom !\n");
		end(EMPTY_NAME_ERROR);
	}
	line += i;
	line = noblanknorline(line);
	line = descpart(line);
	writetext(AFTER_NAME_HEAD);
	return line;
}

char* descpart(char* line){
	if(line[0] == SEPARATOR_1) {
		line = writetexthtml(BEFORE_DESC, line+1, AFTER_DESC); //Ecriture de la description
		if(line[0] == SEPARATOR_1) line = writetexthtml(BEFORE_PART, line+1, AFTER_PART); //Ecriture de la particularité
	}
	return line;
}

char* argument(char* line){
	line = writetexthtml(BEFORE_ARGUMENT_VARIABLE, line+1, AFTER_ARGUMENT_VARIABLE); //Ecriture de la variable
	if(line[0] == SEPARATOR_1) {
		line = writetexthtml(BEFORE_ARGUMENT_TYPE, line+1, AFTER_ARGUMENT_TYPE); //Ecriture du type
		line = descpart(line);
	}
	else writetext(AFTER_ARGUMENT_WITHOUT_TYPE);
	if(line[0] == SEPARATOR_2) {
		writetext(BEFORE_VALEUR_LIST);
		do {
			line = writetexthtml(BEFORE_VALEUR, line+1, AFTER_VALEUR); //Eciture d'une valeur possible
			line = descpart(line);
			writetext(AFTER_VALEUR_ELEMENT);
		} while(line[0] == SEPARATOR_2);
		writetext(AFTER_VALEUR_LIST);
	}
	return line;
}

char* example(char* line){
	line = noblanknorline(line);
	int  i=0;
	int  lastchar=0;
	writetext(BEFORE_EXEMPLE);
	while(line[i] != SEPARATOR_1 && line[i+1] != SEPARATOR_1) {
		if(line[i] == NEWLINE) {
			writenchar(line, lastchar+1);
			line = nextline();
			if(line[0] == SEPARATOR_1 && line[1] == SEPARATOR_1) {
				writetext(AFTER_EXEMPLE);
				return 	line = line+2;
			}
			writetext("\n");
			i=0;
			lastchar=0;
			
		}
		if(line[i] != SPACE && line[i] != TAB) lastchar = i;
		i++;
	}
	writenchar(line, lastchar+1);
	writetext(AFTER_EXEMPLE);
	return 	line = line+i+3;
}

char* img(char* line) {
	line = noblanknorline(line);
	int  i=0;
	int  lastchar=0;
	while(line[i] != NEWLINE) {
		if(line[i] != SPACE && line[i] != TAB) lastchar = i;
		i++;
	}
	writetextnchar(BEFORE_IMG, line, lastchar+1, AFTER_IMG);
	return nextline();
}

char* writehtml(char* line) {
	line = noblanknorline(line);
	int  i=0;
	int  lastchar = -1;
	while(line[i] != SEPARATOR_1 && line[i] != SEPARATOR_2) {
		if(line[i] == NEWLINE) {
			if (lastchar >= 0) writenchar(line, lastchar+1);
			line = nextline();
			if(line[0] != SEPARATOR_1 && line[0] != SEPARATOR_2 && !checktag(line)) writenewline();
			else return line;
			i=0;//Réinitialise l'itérateur comme c'est une nouvelle ligne
			lastchar=-1;//Idem pour le dernier caractère utile
			
		}
		if(line[i] != SPACE && line[i] != TAB) lastchar = i;
		i++;
	}
	if (lastchar >= 0) writenchar(line, lastchar+1);
	return line+i;
}

char* nextline() {
	char* line = realline;
	if(fgets(line, BUFFER_SIZE, rstream) == NULL) {
		perror("Erreur ou fin du fichier ");
		end(FGETS_ERROR);
	}
	line = noblank(line);
	return line;
}

char* noblank(char* line) {
	while(line[0] == SPACE || line[0] == TAB) line++;
	return line;
}

char* noblanknorline(char* line) {
	line = noblank(line);
	while (line[0] == NEWLINE) {
		line = nextline();
		line = noblank(line);
	}
	return line;
}

void writetext(char* text) {
	if(fputs(text, wstream) == EOF) {
		perror("Erreur d'écriture ");
		end(FPUTS_ERROR);
	}
}

void writenchar(char* line, int  n) {
	if(!fwrite(line, n, 1, wstream)) {
		perror("Erreur d'écriture ");
		end(FWRITE_ERROR);
	}
}

void writenewline() {
	writetext(HTML_NEWLINE);
}

void writetextnchar(char* begin, char* line, int  n, char* end) {
	writetext(begin);
	writenchar(line, n);
	writetext(end);
}

char* writetexthtml(char* begin, char* line, char* end) {
	writetext(begin);
	line = writehtml(line);
	writetext(end);
	return line;
}

char* writetextdescpart(char* begin, char* line, char* end) {
	writetext(begin);
	line = descpart(line);
	writetext(end);
	return line;
}

int argclose(int argopen) {
	if(argopen) writetext(AFTER_ARGUMENT_LIST);
	return 0;
}

char* init(char* source, char* destination) {
	//Fichier source
	rstream = fopen(source, "r");
	if(rstream == NULL) {
		perror("Erreur d'ouverture du fichier source en lecture ");
		exit(FOPEN_SOURCE_ERROR);	
	}
	
	//Fichier html de destination
	wstream = fopen(destination, "w");
	if(wstream == NULL) {
		fclose(wstream);
		perror("Erreur d'ouverture du fichier html de destination en écriture ");
		exit(FOPEN_DESTINATION_ERROR);
	}
	
	//Buffer contenant la ligne courante
	char* line;
	line = malloc(sizeof(*line) * BUFFER_SIZE);
	if(line == NULL) {
		perror("Erreur d'allocation du buffer ");
		end(MALLOC_ERROR);
	 }
	realline = line;
	return line;
}

void end(int code) {
	fclose(rstream);
	fclose(wstream);
	exit(code);
}
