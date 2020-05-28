#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "TextToHtml.hpp"

class FileReader {
public:
    FileReader(std::string tf, std::string cf);
	~FileReader();
    
    void readFileText();
    std::string readFileCss();

    void readVector();

private:

    void loadFile(const std::string& fileName);

    std::string Text_File;
    std::string Css_File;
    TextToHtml html;
    std::vector<std::string> text;
};

#endif