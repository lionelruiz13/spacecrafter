#include "include/BitCluster.hpp"

#include <iostream>
BitCluster::BitCluster(unsigned int max_size, u_char maxWriteSize)
{
    // define write tier limit
    for (maxWriteSize--; maxWriteSize > 0; maxWriteSize = maxWriteSize >> 1)
        // Count number of bits needed
        bitMaskSize++;
    bitMask = (1 << bitMaskSize) - 1;
    cluster.resize((max_size * (8 + bitMaskSize)) / 8 + 4);
    std::fill(cluster.begin(), cluster.end(), 0);
    writePos = cluster.data();
    readPos = writePos;
}

BitCluster::~BitCluster()
{
    cluster.clear();
}

unsigned int BitCluster::read()
{
    const int data = (*((int *) readPos) >> subReadPos);
    const u_char nbBits = data & bitMask;

    subReadPos += nbBits + bitMaskSize;
    readPos += subReadPos >> 3; // similar to subReadPos / 8
    subReadPos &= 7; // similar to subWritePos % 8

    return ((data >> bitMaskSize) & ((1 << nbBits) - 1));
}

void BitCluster::write(unsigned int nb)
{
    u_char nbBits = 0;

    for (int n = nb; n > 0; n = n >> 1)
        // Count number of bits needed
        nbBits++;
    *((int *) writePos) |= (nbBits | (nb << bitMaskSize)) << subWritePos; // insert data into cluster
    //std::cout << (char) (nb + '0');
    subWritePos += bitMaskSize + nbBits;
    writePos += subWritePos >> 3; // similar to subReadPos / 8
    subWritePos &= 7; // similar to subWritePos % 8
}

void BitCluster::resize(unsigned int max_size)
{
    int read_size = readPos - cluster.data();
    int write_size = writePos - cluster.data();

    cluster.resize(max_size + 3);
    readPos = cluster.data() + read_size;
    writePos = cluster.data() + write_size;
}

std::unique_ptr<std::vector<u_char>> BitCluster::getBuffer()
{
    size = writePos + (subWritePos > 0) - cluster.data();
    return (std::make_unique<std::vector<u_char>>(cluster));
}

/*
void BitCluster::setBuffer(std::vector<u_char> &buffer)
{
    cluster.assign(buffer.begin(), buffer.end());
}
//*/

void BitCluster::assign(std::vector<u_char>::iterator begin, std::vector<u_char>::iterator end)
{
    cluster.assign(begin, end);
    cluster.resize(cluster.size() + 3); // we must have at least 3 more bytes to ensure we can read as integer
    readPos = cluster.data();
    writePos = cluster.data();
    subReadPos = subWritePos = 0;
}
