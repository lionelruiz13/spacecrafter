#include "include/BitCluster.hpp"

BitCluster::BitCluster(unsigned int max_size, char maxWriteSize)
{
    cluster = new char [max_size];
    std::fill(cluster, cluster + max_size, 0);
    writePos = cluster;
    readPos = cluster;

    // define write tier limit
    for (maxWriteSize--; maxWriteSize > 0; maxWriteSize >> 1)
        // Count number of bits needed
        bitMaskSize++;
    bitMask = (1 << bitMaskSize) - 1;
}

BitCluster::~BitCluster()
{
    delete cluster;
}

unsigned int BitCluster::read()
{
    const int data = (*((int *) readPos) >> subReadPos);
    const char nbBits = data & bitMask;

    subReadPos += nbBits + bitMaskSize;
    readPos += subReadPos >> 8; // 1 octect -> 8 bits
    subReadPos &= 255; // 1111 1111 in binary
    return ((data >> bitMaskSize) & ((1 << nbBits) - 1));
}

void BitCluster::write(unsigned int nb)
{
    char nbBits = 0;

    for (int n = nb; n > 0; n >> 1)
        // Count number of bits needed
        nbBits++;
    *((int *) writePos) |= (nbBits | (nb << bitMaskSize)) << subWritePos; // insert data into cluster
    subWritePos += bitMaskSize + nbBits;
    writePos += subWritePos >> 8; // 1 octect -> 8 bits
    subWritePos &= 255; // 1111 1111 in binary
}
