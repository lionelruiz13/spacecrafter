#pragma once

#include <vector>
#include <string>
#include "DataCompress.hpp"

class PImage
{
public:
    u_int getHeigh() {return heigh;};
    u_int getWidth() {return width;};
    void saveToFile(std::string filename, char format);
    void loadFromFile(std::string filename);
    char getFormat() {return format;};
protected:
    void readProperties(std::ifstream &file);
    void writeProperties(std::ofstream &file, char format);
    char format = -1;
    u_int width;
    u_int heigh;
    u_short nbColor;
    DataCompress datas;
};
