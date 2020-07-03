#pragma once

#include "PImage.hpp"

class P5Image : public PImage
{
public:
    P5Image(PImage &image);
    P5Image(std::ifstream &file);
    virtual ~P5Image();
    virtual void saveToFile(std::string filename);
private:
    char format;
};
