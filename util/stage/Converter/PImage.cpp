#include "../include/dynamic_printer.h"
#include "include/PImage.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <assert.h>

void PImage::loadFromFile(std::string filename)
{
    std::ifstream file (filename, std::ifstream::binary);

    if (file) {
        char buffer[3] = "__";
        file.read(buffer, 3);
        assert(buffer[0] == 'P' && buffer[2] == '\n');
        readProperties(file);

        std::vector<u_char> tmp;
        { // to optimize
            char buff[4096];
            int tmpsize = 0;
            while ((tmpsize = file.readsome(buff, 4096)) == 4096)
                tmp.insert(tmp.end(), buff, buff + 4096);
            tmp.insert(tmp.end(), buff, buff + tmpsize);
        }
        switch (buffer[1]) {
            case '5':
                format = 5;
                datas.set(tmp);
                break;
            case '8':
                format = 8;
                datas.setCompressed(tmp, width * heigh);
                break;
            default:
                format = -1;
                my_set_color(FOREGROUND_DARK + RED);
                std::cout << "Error : Invalid input file." << std::endl;
                my_set_effect(CLEAR);
        }
        file.close();
    }
}

void PImage::saveToFile(std::string filename, char format)
{
    if (this->format == -1) {
        my_set_color(FOREGROUND_DARK + RED);
        std::cout << "Error : Can't save empty image." << std::endl;
        my_set_effect(CLEAR);
        return;
    }

    std::ofstream file (filename, std::ofstream::binary);

    if (file) {
        writeProperties(file, format);
        std::unique_ptr<std::vector<u_char>> tmp;
        switch (format) {
            case 5:
                tmp = datas.get();
                break;
            case 8:
                tmp = datas.getCompressed();
                break;
            default:
                my_set_color(FOREGROUND_DARK + RED);
                std::cout << "Error : Invalid format." << std::endl;
                my_set_effect(CLEAR);
                file.close();
                return;
        }
        file.write((char *) tmp->data(), tmp->size()); // because (unsigned char *) can be used as (char *) safely.
        file.close();
    }
}

void PImage::readProperties(std::ifstream &file)
{
    char buffer[64];

    do {
        file.getline(buffer, 64);
    } while (buffer[0] == '#');
    width = std::stoi(buffer);
    short pos = -1;
    while (buffer[++pos] != ' ');
    heigh = std::stoi(buffer + pos);

    do {
        file.getline(buffer, 64);
    } while (buffer[0] == '#');
    nbColor = std::stoi(buffer);
}

void PImage::writeProperties(std::ofstream &file, char format)
{
    file << "P" << (int) format << std::endl << width << " " << heigh << std::endl << nbColor << std::endl;
}
