#include "include/DataCompress.hpp"
#include <algorithm>

void DataCompress::set(std::vector<u_char> &datas)
{
    std::vector<std::pair<short, u_char>> myDict(256);

    std::for_each(myDict.begin(), myDict.end(), [](std::pair<short, u_char> &value) {
        static u_char i = 0;
        value.first = 0;
        value.second = i++;
    });

    for (auto& value: datas) {
        myDict[value].first++; // Increase occurency number of corresponding pixel bytecode
    }

    myDict.erase(std::remove_if(myDict.begin(), myDict.end(), [](std::pair<short, u_char> &value) {return value.first == 0;}));

    std::sort(myDict.begin(), myDict.end(), [](std::pair<short, u_char> &value1, std::pair<short, u_char> &value2) {
        return (value1.first > value2.first);
    });

    std::vector<u_char> tmp(256);
    dict.resize(myDict.size());
    for (u_char key = 0; key < myDict.size(); key++) {
        dict[key] = myDict[key].second;
        tmp[myDict[key].second] = key;
    }
    cluster = std::make_unique<BitCluster>(datas.size(), myDict.size());
    for (auto& value: datas) {
        cluster->write(tmp[value]);
    }
}

void DataCompress::get(std::vector<u_char> &datas)
