#ifndef _STRING_ID_HPP_
#define _STRING_ID_HPP_

#include <set>
#include <deque>
#include <string>
#include <string_view>
#include <cstring>

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
        // Lookup with the caller's live bytes (safe: comparisons only read
        // STORED entries, which are owned below), then intern a COPY on miss.
        // The original stored name.data() directly - a caller passing a
        // temporary std::string (explicit slot names: the GRID call,
        // composed-format slot=) left a DANGLING pointer that every future
        // same-size insert memcmp'd against (latent UB, found at the B24
        // composed-slot extension; fixed at the class - I6).
        const StringID probe{name.data(), static_cast<uint32_t>(name.size()),
                             static_cast<uint32_t>(entries.size())};
        const auto it = entries.find(probe);
        if (it != entries.end())
            return *it;
        const std::string &owned = names.emplace_back(name);
        return *entries.insert({owned.data(), probe.size, probe.id}).first;
    }
    // Reverse lookup (id -> interned name), O(entries) - instrumentation use
    // (slot-inventory dump), never a hot path. Empty view on an unknown id.
    std::string_view nameOf(uint32_t id) const
    {
        for (const auto &e : entries)
            if (e.id == id)
                return {e.name, e.size};
        return {};
    }
private:
    std::set<StringID> entries;
    std::deque<std::string> names; // owns the interned bytes (deque: stable addresses)
};

#endif
