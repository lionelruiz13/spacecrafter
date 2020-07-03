#pragma once

#include <memory>
#include <array>
#include <vector>

class PImage
{
public:
    u_int getHeigh() {return heigh;};
    u_int getWidth() {return width;};
    const std::vector<char> getRawImage() {return rawImage;};
    virtual void saveToFile(std::string filename) = 0;
    static PImage *loadFromFile(std::string filename);
protected:
    void readProperties(std::ifstream);
    u_int width;
    u_int heigh;
    u_short nbColor;
    std::vector<char> rawImage;
};

class P5Image : public PImage
{
public:
    P5Image(PImage &image);
    P5Image(std::ifstream file);
    virtual ~P5Image();
    virtual void saveToFile(std::string filename);
private:
    char format;
};

class P8Image : public PImage
{
public:
    P8Image(PImage &image);
    P8Image(std::ifstream file);
    virtual ~P8Image();
    virtual void saveToFile(std::string filename);
private:
    void compressRawImage();
    char *colorDict;
    char *compressedImage;
    char nbColor;
    char format;
};
