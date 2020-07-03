#include "include/P8Image.hpp"
#include "include/BitCluster.hpp"
#include <array>

P8Image::P8Image(std::ifstream &file) : format(8)
{
    readProperties(file);
}

P8Image::~P8Image()
{}

void P8Image::saveToFile(std::string filename)
{}

void P8Image::compressRawImage()
{
    std::array<char, 256> nbOccurency;

    nbOccurency.fill(0);

    for (int i = 0; i < length;) {
        nbOccurency[rawImage[i++]]++; // Increase occurency number of corresponding pixel bytecode
    }
}
