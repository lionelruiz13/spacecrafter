#pragma once

#include <vector>
#include <string>

class PImage
{
public:
    u_int getHeigh() {return heigh;};
    u_int getWidth() {return width;};
    const std::vector<char> getRawImage() {return rawImage;};
    virtual void saveToFile(std::string filename) = 0;
    static PImage *loadFromFile(std::string filename);
protected:
    void readProperties(std::ifstream &file);
    u_int width;
    u_int heigh;
    u_int length;
    u_short nbColor;
    std::vector<char> rawImage;
};
