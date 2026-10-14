// Return the name the two cache names are cut from
static std::string cacheSlice(const std::string &name)
{
    const auto pos = name.find(".spacecrafter/");
    if (pos == std::string::npos && CallSystem::isAbsolute(name))
        return name;
    const std::string rel = (pos == std::string::npos) ? name : name.substr(pos + 14);
    const auto slash = rel.find('/');
    return (slash == std::string::npos) ? rel : rel.front() + rel.substr(slash);
}

std::string getCacheName(const std::string &name)
{
    const std::string slice = cacheSlice(name);
    std::string filename = slice.substr(0, slice.size() - 3) + "dat";
    if (filename.front() == '/')
        filename.front() = '_';
    std::replace(filename.begin(), filename.end(), '/', '-');
    return filename;
}

std::string getCacheEntryName(const std::string &name)
{
    const std::string slice = cacheSlice(name);
    return slice.substr(0, slice.size() - 4);
}
