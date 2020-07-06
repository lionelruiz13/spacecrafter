#pragma once

#include <vector>
#include <memory>

typedef unsigned char u_char;

class BitCluster
{
public:
    BitCluster(unsigned int max_size, u_char maxWriteSize = 255);
    ~BitCluster();
    void resetReadPos() {readPos = cluster.data(); subReadPos = 0;};
    unsigned int read();
    void write(unsigned int nbr);
    std::unique_ptr<std::vector<u_char>> getBuffer();
    void setBuffer(std::vector<u_char> &buffer);
    const unsigned int getSize() {return (writePos - cluster.data());};
    const unsigned int getMaxSize() {return cluster.size();}
    void resize(unsigned int max_size);
private:
    u_char *readPos; // cluster reading point
    u_char *writePos; // cluster writing point
    std::vector<u_char> cluster;
    u_char subReadPos = 0;
    u_char subWritePos = 0;
    u_char bitMaskSize = 0; // 3 -> 2^3 = 8 bits max
    u_char bitMask = 0;
};
