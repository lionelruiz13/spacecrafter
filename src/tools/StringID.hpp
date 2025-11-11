#ifndef _STRING_ID_HPP_
#define _STRING_ID_HPP_

#include <set>
#include <string_view>

struct StringID
{
    bool operator<(const StringID &other) const
    {
        if (size != other.size)
            return size < other.size;
        return memcmp(name, other.name, size) < 0;
    }
    bool operator==(const StringID &other) const
    {
        return other.id == id;
    }

    const char *name;
    uint32_t size;
    uint32_t id;
};

class StringIDCluster
{
public:
    const StringID &operator[](std::string_view name)
    {
        return *entries.insert({name.data(), static_cast<uint32_t>(name.size()), static_cast<uint32_t>(entries.size())}).first;
    }
private:
    std::set<StringID> entries;
};

#endif
