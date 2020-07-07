#include "include/DataCompress.hpp"
#include <algorithm>
#include <iostream>

void DataCompress::set(std::vector<u_char> &datas)
{
    std::vector<std::pair<int, u_char>> myDict(256);
    // (priority, color)
    size = datas.size();

    std::for_each(myDict.begin(), myDict.end(), [](std::pair<int, u_char> &value) {
        static u_char i = 0;
        value.first = 0; // set priority to 0
        value.second = i++; // set index as color
    });

    for (auto& value: datas) {
        myDict[value].first++; // Increase priority of corresponding color
    }

    myDict.erase(std::remove_if(myDict.begin(), myDict.end(), [](std::pair<int, u_char> &value) {return value.first == 0;}), myDict.end());

    std::sort(myDict.begin(), myDict.end(), [](std::pair<int, u_char> &value1, std::pair<int, u_char> &value2) {
        return (value1.first > value2.first);
    });

    std::vector<u_char> tmp(256);
    dict.resize(myDict.size());
    for (u_char key = 0; key < myDict.size(); key++) {
        dict[key] = myDict[key].second;
        tmp[myDict[key].second] = key;
        std::cout << "key " << (int) key << " represent " << (int) dict[key] << " occuring " << (int) myDict[key].first << " times." << std::endl;
        std::cout << "convert " << (int) myDict[key].second << " into " << (int) tmp[myDict[key].second] << std::endl;
    }
    cluster = std::make_unique<BitCluster>(size, myDict.size());
    for (auto& value: datas) {
        cluster->write(tmp[value]);
    }
}

std::unique_ptr<std::vector<u_char>> DataCompress::get()
{
    std::unique_ptr<std::vector<u_char>> datas = std::make_unique<std::vector<u_char>>(size);

    cluster->resetReadPos();
    std::generate(datas->begin(), datas->end(), [this]() {return this->dict[this->cluster->read()];});

    return std::move(datas);
}

void DataCompress::setCompressed(std::vector<u_char> &datas, int nb_datas)
{
    std::vector<u_char>::iterator it = datas.begin() + 1;
    dict.assign(it, it + datas[0]);
    cluster = std::make_unique<BitCluster>(0, datas[0]);
    cluster->assign(it + datas[0], datas.end()); // issue here
    size = nb_datas;
}

std::unique_ptr<std::vector<u_char>> DataCompress::getCompressed()
{
    auto datas = std::make_unique<std::vector<u_char>>(1);

    (*datas)[0] = dict.size();

    datas->insert(datas->end(), dict.begin(), dict.end());

    auto tmp = cluster->getBuffer();
    datas->insert(datas->end(), tmp->begin(), tmp->begin() + cluster->getSize());

    return (std::move(datas));
}
