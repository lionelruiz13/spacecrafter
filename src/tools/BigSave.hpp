/*
** EPITECH PROJECT, 2021
** Global Tools
** File description:
** BigSave.hpp
*/

#ifndef BIG_SAVE_HPP_
#define BIG_SAVE_HPP_

#include <string>
#include <map>
#include <vector>
#include <exception>

enum SaveSection {
    UNDEFINED,
    STRING_MAP = 0x01,
    ADDRESS_MAP = 0x02,
    LIST = 0x03,
    CHAR_SIZE = 0x04,
    SHORT_SIZE = 0x08,
    INT_SIZE = 0x0c,
};

#define TYPE_MASK 0x03
#define SIZE_MASK 0x0c

class SaveData {
public:
    SaveData();
    ~SaveData();

    SaveData &operator[](const std::string &key);
    SaveData &operator[](unsigned long address);
    int push(const SaveData &data = {}); // return new index
    int push(const std::string &data); // return new index
    // Warning : Omit destructor of T
    template<typename T>
    int push(const T &data) {
        SaveData tmp;
        tmp.raw.resize(sizeof(T));
        *reinterpret_cast<T *>(tmp.raw.data()) = data;
        return push(tmp);
    }
    int pushData(const std::vector<char> &data);
    std::vector<SaveData> &getList() {return arr;}
    void truncate(); // Discard content attached to it (except raw)
    void reset(); // Discard content, type and attached datas
    SaveSection getType() const {return (SaveSection) type;}
    bool nonEmpty() const;
    bool empty() const;
    std::vector<char> &get() {return raw;}
    // Warning : Omit destructor of T
    template<typename T>
    T &get(const T &defaultValue = {}) {
        if (raw.empty()) {
            raw.resize(sizeof(T));
            *reinterpret_cast<T *>(raw.data()) = defaultValue;
        }
        return *reinterpret_cast<T *>(raw.data());
    }
    void load(char *&data);
    void save(std::vector<char> &data);
    size_t computeSize();
private:
    void save(char *data);
    inline size_t getSize() const {return dataSize;}
    unsigned char type = SaveSection::UNDEFINED;
    unsigned char sizeType = SaveSection::UNDEFINED;
    size_t dataSize;

    std::map<std::string, SaveData> str;
    std::map<unsigned long, SaveData> addr;
    std::vector<SaveData> arr;

    std::vector<char> raw;
};

class BigSave : public SaveData {
public:
    BigSave();
    ~BigSave();

    // _saveAtDestroy : If true, call store() when this object is destroyed
    // reduceWrite : If true, when calling store(), read the file and compare it with the new content. If they are equal, don't write to the file.
    // reducedCheck : If true, assume that content is unchanged if this->get() content is unchanged.
    bool open(const std::string &saveName, bool _saveAtDestroy = true, bool _reduceWrite = true, bool _reducedCheck = false);
    bool store();
    const std::string &getSaveName() const;
private:
    std::string saveName;
    std::vector<char> oldData; // reducedCheck == true, store the attached content at the last open() or store() call
    bool saveAtDestroy;
    bool reduceWrite;
    bool reducedCheck;
};

#endif /* BIG_SAVE_HPP_ */
