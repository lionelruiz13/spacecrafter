#include <string>
#include <iostream>

#include "TextToHtml.hpp"
#include "FileReader.hpp"
#include "FileWriter.hpp"
#include "DefineFile.hpp"

int main() {

    FileReader* Reader = new FileReader();
    FileWriter* Writer = new FileWriter();
    TextToHtml* Text = new TextToHtml();

    std::string line;

    std::string source = "input_fr.txt";
    std::string sourceCSS = "style.css";
    std::string destination = "resultat.html";

    line = Reader->init(source, destination);
    //Fichier de style
    FILE* rstylestream = fopen(sourceCSS.c_str(), "r");
    if(rstylestream == NULL) {
        perror("Erreur d'ouverture du fichier style en lecture ");
        exit(FOPEN_STYLE_ERROR);	
    }
    Writer->writetext(BEFORE_STYLE);
    Writer->writetext("<style>\n");
	char *cstr = new char[line.length() + 1];
    while(fgets(strcpy(cstr, line.c_str()), BUFFER_SIZE, rstylestream) != NULL) Writer->writetext(line);
    Writer->writetext("\n</style>\n");
    fclose(rstylestream);
			

	std::string* nametab;//Pointeur sur le tableau de noms de commandes
	Writer->writetext(AFTER_STYLE);
	line = Reader->nextline();//Passe à la première ligne
	int  blockopen = 0;//Témoin de block ouvert
	int argopen = 0;//Témoin de liste d'arguments ouverte
	while(1) {
		switch(Text->checktag(line)) {
			case NAME :
				argopen =  Writer->argclose(argopen);
				if(blockopen) Writer->writetext(AFTER_BLOCK);
				else {
					//nametab = malloc(sizeof(*nametab) * NAME_TAB_SIZE);
					if(line == "") {
						perror("Erreur d'allocation du tableau de noms : ");
						Reader->end(MALLOC_ERROR);
					 }
				}
				line = Text->name(line, nametab, blockopen);
				blockopen++;
				break;
				
			case ARGUMENT :
				if(!argopen) {
					Writer->writetext(BEFORE_ARGUMENT_LIST);
					argopen++;
				}
				line = Text->argument(line);
				break;
				
			case EXEMPLE :
				argopen =  Writer->argclose(argopen);
				line = Text->example(line);
				break;
				
			case IMG :
				line = Text->img(line);
				break;
				
			case END :
				 Writer->argclose(argopen);
				if(blockopen) {
					Writer->writetext(AFTER_BLOCK);
					Writer->writetext(BEFORE_MENU);
					int  i;
					for(i=0; i < blockopen; i++) {
						Writer->writetext(BEFORE_MENU_NAME);
						Writer->writetext(nametab[i]);
						Writer->writetext(AFTER_MENU_LINK);
						Writer->writetext(nametab[i]);
						Writer->writetext(AFTER_MENU_NAME);
					}
					Writer->writetext(AFTER_MENU);
				}
				Writer->writetext(FOOT);
				Reader->end(0);
				
			default : line = Reader->nextline();
		}
	}

    return 0;
}