/*
** EPITECH PROJECT, 2021
** Global Tools
** File description:
** BigSave.cpp
*/
#include "BigSave.hpp"
#include <fstream>
#include <iostream>
#include <cstring>
#include <functional>

BigSave::BigSave()
{}

BigSave::~BigSave()
{
    if (saveAtDestroy)
        store();
}

bool BigSave::open(const std::string &name, bool _saveAtDestroy, bool _reduceWrite, bool _reducedCheck)
{
    std::ifstream file(name + ".sav", std::ifstream::binary);
    size_t size;
    std::vector<char> data;

    saveName = name;
    saveAtDestroy = _saveAtDestroy;
    reduceWrite = _reduceWrite;
    reducedCheck = _reducedCheck;
    if (!file || !file.read((char *) &size, sizeof(size_t)))
        return false;

    data.resize(size);
    if (!file.read(data.data(), size)) {
        std::cerr << "Warning : Save file is truncated or corrupted\n";
        return false;
    }
    char *pData = data.data();
    load(pData);
    if (pData != data.data() + data.size()) {
        throw std::range_error("Theoric size and bloc size missmatch");
    }
    if (reducedCheck)
        oldData = get();
    return true;
}

bool BigSave::store()
{
    std::vector<char> data;
    auto flags = std::ofstream::binary | std::ofstream::trunc;
    if (reduceWrite) {
        if (reducedCheck) {
            if (oldData.size() == get().size() && std::memcmp(oldData.data(), get().data(), oldData.size()) == 0)
                return true; // Assume nothing has changed
            oldData = get();
        }
        std::ifstream file(saveName + ".sav", std::ifstream::binary);
        if (file) {
            save(data);
            size_t oldSize;
            file.read((char *) &oldSize, sizeof(oldSize));
            if (oldSize <= data.size()) {
                flags = std::ofstream::binary;
                if (!reducedCheck && oldSize == data.size()) {
                    std::vector<char> oldData;
                    oldData.resize(data.size());
                    file.read(oldData.data(), data.size());
                    if (std::memcmp(data.data(), oldData.data(), data.size()) == 0)
                        return true;
                }
            }
        }
    }
    std::ofstream file(saveName + ".sav", flags);
    if (file) {
        if (data.empty())
            save(data);
        const size_t size = data.size();
        file.write((char *) &size, sizeof(size));
        file.write(data.data(), size);
    }
    return (file.good());
}

const std::string &BigSave::getSaveName() const
{
    return saveName;
}

SaveData::SaveData() {}
SaveData::~SaveData() {}

SaveData &SaveData::operator[](const std::string &key)
{
    switch (type) {
        case SaveSection::UNDEFINED:
            type = SaveSection::STRING_MAP;
            [[fallthrough]];
        case SaveSection::STRING_MAP:
            break;
        default:
            throw std::bad_function_call();
    }
    return str[key];
}

SaveData &SaveData::operator[](unsigned long address)
{
    switch (type) {
        case SaveSection::UNDEFINED:
            type = SaveSection::ADDRESS_MAP;
            [[fallthrough]];
        case SaveSection::ADDRESS_MAP:
            break;
        case SaveSection::LIST:
            return arr[address];
        default:
            throw std::bad_function_call();
    }
    return addr[address];
}

int SaveData::push(const SaveData &data)
{
    switch (type) {
        case SaveSection::UNDEFINED:
            type = SaveSection::LIST;
            [[fallthrough]];
        case SaveSection::LIST:
            arr.push_back(data);
            break;
        default:
            throw std::bad_function_call();
    }
    return arr.size() - 1;
}

int SaveData::push(const std::string &data)
{
    switch (type) {
        case SaveSection::UNDEFINED:
            type = SaveSection::LIST;
            [[fallthrough]];
        case SaveSection::LIST:
        {
            SaveData sd;
            sd.get().resize(data.size());
            memcpy(sd.get().data(), data.c_str(), data.size());
            arr.push_back(sd);
            break;
        }
        default:
            throw std::bad_function_call();
    }
    return arr.size() - 1;
}

int SaveData::pushData(const std::vector<char> &data)
{
    switch (type) {
        case SaveSection::UNDEFINED:
            type = SaveSection::LIST;
            [[fallthrough]];
        case SaveSection::LIST:
        {
            SaveData sd;
            sd.get().resize(data.size());
            memcpy(sd.get().data(), data.data(), data.size());
            arr.push_back(sd);
            break;
        }
        default:
            throw std::bad_function_call();
    }
    return arr.size() - 1;
}

void SaveData::truncate()
{
    str.clear();
    addr.clear();
    arr.clear();
}

void SaveData::reset()
{
    truncate();
    raw.clear();
    type = SaveSection::UNDEFINED;
}

bool SaveData::nonEmpty() const
{
    if (raw.empty()) {
        switch (type) {
            case SaveSection::UNDEFINED:
                return false;
            case SaveSection::STRING_MAP:
                return !str.empty();
            case SaveSection::ADDRESS_MAP:
                return !addr.empty();
            case SaveSection::LIST:
                return !arr.empty();
        }
    }
    return true;
}

bool SaveData::empty() const
{
    if (raw.empty()) {
        switch (type) {
            case SaveSection::UNDEFINED:
                return true;
            case SaveSection::STRING_MAP:
                return str.empty();
            case SaveSection::ADDRESS_MAP:
                return addr.empty();
            case SaveSection::LIST:
                return arr.empty();
        }
    }
    return false;
}

void SaveData::load(char *&data)
{
    size_t size = 0;
    type = *(data++);
    sizeType = type & SIZE_MASK;
    type &= TYPE_MASK;
    switch (sizeType) {
        case SaveSection::CHAR_SIZE:
            size = *((uint8_t *&) data)++;
            break;
        case SaveSection::SHORT_SIZE:
            size = *((uint16_t *&) data)++;
            break;
        case SaveSection::INT_SIZE:
            size = *((uint32_t *&) data)++;
            break;
        default:
            goto NO_DATA; // There is no attached data
    }
    raw.resize(size);
    memcpy(raw.data(), data, size);
    data += size;
    NO_DATA:
    switch (type) {
        case SaveSection::UNDEFINED:
            break;
        case SaveSection::STRING_MAP:
        {
            uint16_t nbEntry = *(((uint16_t *&) data)++);
            while (nbEntry--) {
                std::string s(data + 1, *data);
                data += *((unsigned char *) data) + 1;
                str[s].load(data);
            }
            break;
        }
        case SaveSection::ADDRESS_MAP:
        {
            uint16_t nbEntry = *(((uint16_t *&) data)++);
            while (nbEntry--)
                addr[*((uint64_t *&) data)++].load(data);
            break;
        }
        case SaveSection::LIST:
        {
            uint16_t nbEntry = *(((uint16_t *&) data)++);
            arr.resize(nbEntry);
            for (uint16_t i = 0; i < nbEntry; ++i)
                arr[i].load(data);
            break;
        }
    }
}

void SaveData::save(char *data)
{
    *(data++) = type | sizeType;
    switch (sizeType) {
        case SaveSection::CHAR_SIZE:
            *(((uint8_t *&) data)++) = raw.size();
            break;
        case SaveSection::SHORT_SIZE:
            *(((uint16_t *&) data)++) = raw.size();
            break;
        case SaveSection::INT_SIZE:
            *(((uint32_t *&) data)++) = raw.size();
            break;
    }
    memcpy(data, raw.data(), raw.size());
    data += raw.size();
    switch (type) {
        case SaveSection::UNDEFINED:
            break;
        case SaveSection::STRING_MAP:
        {
            uint16_t &nbEntry = *((uint16_t *&) data)++;
            nbEntry = 0;
            for (auto &v : str) {
                if (v.second.nonEmpty()) {
                    ++nbEntry;
                    *(((uint8_t *&) data)++) = v.first.size();
                    memcpy(data, v.first.c_str(), v.first.size());
                    data += v.first.size();
                    v.second.save(data);
                    data += v.second.getSize();
                }
            }
            break;
        }
        case SaveSection::ADDRESS_MAP:
        {
            uint16_t &nbEntry = *((uint16_t *&) data)++;
            nbEntry = 0;
            for (auto &v : addr) {
                if (v.second.nonEmpty()) {
                    ++nbEntry;
                    *(((uint64_t *&) data)++) = v.first;
                    v.second.save(data);
                    data += v.second.getSize();
                }
            }
            break;
        }
        case SaveSection::LIST:
        {
            *(((uint16_t *&) data)++) = arr.size();
            for (auto &v : arr) {
                v.save(data);
                data += v.getSize();
            }
            break;
        }
    }
}

size_t SaveData::computeSize()
{
    dataSize = raw.size();
    if (dataSize > 0) {
        if (dataSize > UINT8_MAX) {
            if (dataSize > UINT16_MAX) {
                sizeType = SaveSection::INT_SIZE;
                dataSize += 4;
            } else {
                sizeType = SaveSection::SHORT_SIZE;
                dataSize += 2;
            }
        } else {
            sizeType = SaveSection::CHAR_SIZE;
            dataSize += 1;
        }
    }
    switch (type) {
        case SaveSection::UNDEFINED:
            break;
        case SaveSection::STRING_MAP:
            for (auto &v : str) {
                if (v.second.nonEmpty())
                    dataSize += v.first.size() + 1 + v.second.computeSize();
            }
            dataSize += 2;
            break;
        case SaveSection::ADDRESS_MAP:
            for (auto &v : addr) {
                if (v.second.nonEmpty())
                    dataSize += 8 + v.second.computeSize();
            }
            dataSize += 2;
            break;
        case SaveSection::LIST:
            for (auto &v : arr) {
                dataSize += v.computeSize();
            }
            dataSize += 2;
            break;
    }
    return ++dataSize;
}

void SaveData::save(std::vector<char> &data)
{
    data.resize(computeSize());
    char *ptr = data.data();
    save(ptr);
}
