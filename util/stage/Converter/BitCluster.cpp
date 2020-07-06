#include "include/BitCluster.hpp"

BitCluster::BitCluster(unsigned int max_size, u_char maxWriteSize)
{
    cluster.resize(max_size);
    std::fill(cluster.begin(), cluster.end(), 0);
    writePos = cluster.data();
    readPos = writePos;

    // define write tier limit
    for (maxWriteSize--; maxWriteSize > 0; maxWriteSize >> 1)
        // Count number of bits needed
        bitMaskSize++;
    bitMask = (1 << bitMaskSize) - 1;
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
    readPos += subReadPos >> 8; // 1 octect -> 8 bits
    subReadPos &= 255; // 1111 1111 in binary
    return ((data >> bitMaskSize) & ((1 << nbBits) - 1));
}

void BitCluster::write(unsigned int nb)
{
    u_char nbBits = 0;

    for (int n = nb; n > 0; n >> 1)
        // Count number of bits needed
        nbBits++;
    *((int *) writePos) |= (nbBits | (nb << bitMaskSize)) << subWritePos; // insert data into cluster
    subWritePos += bitMaskSize + nbBits;
    writePos += subWritePos >> 8; // 1 octect -> 8 bits
    subWritePos &= 255; // 1111 1111 in binary
}

void BitCluster::resize(unsigned int max_size)
{
    int read_size = readPos - cluster.data();
    int write_size = writePos - cluster.data();

    cluster.resize(max_size);
    readPos = cluster.data() + read_size;
    writePos = cluster.data() + write_size;
}

std::unique_ptr<std::vector<u_char>> BitCluster::getBuffer()
{
    return (std::make_unique<std::vector<u_char>>(cluster));
}

void BitCluster::setBuffer(std::vector<u_char> &buffer)
{
    cluster.clear();
    cluster = buffer;
}
