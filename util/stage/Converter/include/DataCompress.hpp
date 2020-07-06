#pragma once

#include "BitCluster.hpp"
#include <vector>
#include <memory>

class DataCompress
{
public:
    void set(std::vector<u_char> &datas);
    std::unique_ptr<std::vector<u_char>> get();
    void setCompressed(std::vector<u_char> &data);
    std::unique_ptr<std::vector<u_char>> getCompressed();
private:
    std::vector<u_char> dict;
    std::unique_ptr<BitCluster> cluster = nullptr;
};
