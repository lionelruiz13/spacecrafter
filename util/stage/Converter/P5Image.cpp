#include "include/P5Image.hpp"

P5Image::P5Image(std::ifstream &file) : format(5)
{
    readProperties(file);
}

P5Image::~P5Image()
{}

void P5Image::saveToFile(std::string filename)
{}
