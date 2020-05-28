#ifndef FILE_WRITER_HPP
#define FILE_WRITER_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string>

class FileWriter {
public:
    FileWriter();
	~FileWriter();
    
                                           

private:
    FILE* wstream;      //Flux d'écriture
};

#endif