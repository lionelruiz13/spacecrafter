#ifndef TEXT_TO_HTML_HPP
#define TEXT_TO_HTML_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

class TextToHtml {
public:
    TextToHtml(std::vector<std::string> _text, std::string _css);
	~TextToHtml();

    //On lit le vecteur d'entrée (inText)
    void lecture();

private:
    /**
     * Cette fonction reçois chaque itération du vecteur d'entrée, et traite tout le string.
     * La fonction va découper le string en 4 : NAME / ARGUMENT / PARAMETER / EXEMPLE
     * Puis, elle stock dans outHtml le résultat, pour un traitement dans FileWriter. 
     */
    void transformation(std::string lines);
    /**
     * Fonction de détéction de l'argument courrant : NAME / ARGUMENT / PARAMETER / EXEMPLE
     * pour permettre un découpage précis
     */ 
    std::string findBloc(std::string lines, std::string arg);

    //Procédure de Transformation vers HTML.
    void NameInHtml(std::string lines);
    void ArgumentInHtml(std::string lines);
    void ParameterInHtml(std::string lines);
    void ExempleInHtml(std::string lines);

    //Fichier de Sortie pour FileWriter
    std::string OutHtml;

    //Fichier d'éntrée
    std::vector<std::string> inText;
    std::string inCss;
};

#endif