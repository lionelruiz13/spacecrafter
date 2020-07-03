#pragma once

#include <vector>

class BitCluster
{
public:
    BitCluster(unsigned int max_size, char maxWriteSize = 8);
    ~BitCluster();
    void resetPos() {readPos = cluster; subReadPos = 0;};
    unsigned int read();
    void write(unsigned int nbr);
    const char *getBuffer() {return cluster;};
    const unsigned int getSizeUsed() {return (writePos - cluster);};
private:
    char *readPos; // cluster reading point
    char *writePos; // cluster writing point
    char *cluster;
    char subReadPos = 0;
    char subWritePos = 0;
    char bitMaskSize = 0; // 3 -> 2^3 = 8 bits max
    char bitMask = 0;
};
