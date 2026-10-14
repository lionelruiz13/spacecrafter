#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
struct CallSystem { static bool isAbsolute(const std::string path); };
#include "isabsolute.inc"
namespace today {
#include "today.inc"
}
namespace after {
#include "after.inc"
}
template <class F> static void run(F f, const std::string &n, std::string &st, std::string &v)
{
    try { v = f(n); st = "OK"; } catch (const std::out_of_range &e) { v = e.what(); st = "THROW"; }
}
int main(int argc, char **argv)
{
    const bool t = std::string(argv[1]) == "today";
    std::ifstream in(argv[2], std::ios::binary);
    std::string n, se, e, sc, c;
    while (std::getline(in, n)) {
        if (t) { run(today::getCacheEntryName, n, se, e); run(today::getCacheName, n, sc, c); }
        else { run(after::getCacheEntryName, n, se, e); run(after::getCacheName, n, sc, c); }
        std::cout << n << '\t' << se << '\t' << e << '\t' << sc << '\t' << c << '\n';
    }
}
