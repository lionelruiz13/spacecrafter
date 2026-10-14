std::string getCacheName(const std::string &name)
{
    auto p1 = name.find(".spacecrafter/")+14;
    if (p1 == 13)
        p1 = 0;
    const auto p2 = name.find_first_of('/', p1) - 1;
    std::string filename = name.substr();
    filename = name.substr(p2, name.size() - p2);
    filename.front() = name[p1];
    for (char &c : filename) {
        if (c == '/')
            c = '-';
    }
    memcpy(filename.data() + filename.size() - 3, "dat", 3);
    return filename;
}

std::string getCacheEntryName(const std::string &name)
{
    auto p1 = name.find(".spacecrafter/")+14;
    if (p1 == 13)
        p1 = 0;
    const auto p2 = name.find_first_of('/', p1) - 1;
    std::string filename = name.substr();
    filename = name.substr(p2, name.size() - p2 - 4);
    filename.front() = name[p1];
    return filename;
}
