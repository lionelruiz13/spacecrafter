#include "../include/dynamic_printer.h"
#include "include/PImage.hpp"
#include "include/P5Image.hpp"
#include "include/P8Image.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <assert.h>

PImage *PImage::loadFromFile(std::string filename)
{
    std::ifstream file (filename, std::ifstream::binary);

    if (file) {
        char buffer[3] = "ER";
        file.read(buffer, 3);
        assert(buffer[0] == 'P' && buffer[2] == '\n');

        switch (buffer[1]) {
            case '5':
                return (new P5Image(file));
            case '8':
                return (new P8Image(file));
            default:
                my_set_color(FOREGROUND_DARK + RED);
                std::cout << "Error : Invalid input file." << std::endl;
                my_set_effect(CLEAR);
                file.close();
                return NULL;
        }
    }
}

void PImage::readProperties(std::ifstream &file)
{
    char buffer[64];

    do {
        file.getline(buffer, 64);
    } while (buffer[0] == '#');
    width = std::stoi(buffer);
    char pos = -1;
    while (buffer[++pos] != ' ');
    heigh = std::stoi(buffer + pos);

    do {
        file.getline(buffer, 64);
    } while (buffer[0] == '#');
    nbColor = std::stoi(buffer);
}
