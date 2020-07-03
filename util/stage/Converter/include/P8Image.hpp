#pragma once

#include "PImage.hpp"

class P8Image : public PImage
{
public:
    P8Image(PImage &image);
    P8Image(std::ifstream &file);
    virtual ~P8Image();
    virtual void saveToFile(std::string filename);
private:
    void compressRawImage();
    char *colorDict;
    const char *compressedImage;
    int compressedImageSize;
    char nbColor;
    char format;
};
