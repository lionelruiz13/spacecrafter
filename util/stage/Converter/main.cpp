#include <iostream>
#include "../include/dynamic_printer.h"
#include "include/PImage.hpp"

int main(int argc, char const *argv[])
{
    const char *InputFile;
    const char *OutputFile = "output.pgm";

    switch (argc) {
        case 3:
            OutputFile = argv[2]; __attribute__ ((fallthrough));
        case 2:
            InputFile = argv[1];
            std::cout << "Write converted file as '" << OutputFile << "'." << std::endl;
            break;
        default:
            my_set_color(FOREGROUND_DARK + RED);
            std::cout << "Error : Invalid argument number." << std::endl;
            std::cout << "You must give input filename, and optionnally output filename." << std::endl;
            my_set_effect(CLEAR);
            return 1;
    }

    PImage image;
    image.loadFromFile(InputFile);
    image.saveToFile(OutputFile, image.getFormat() == 5 ? 8 : 5);
    return 0;
}
