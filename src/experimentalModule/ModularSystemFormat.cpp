#include "ModularSystemFormat.hpp"
#include "tools/log.hpp"
#include <fstream>
#include <cstdio>

namespace ModularSystemFormat {

namespace {

inline void trim(std::string &s)
{
    const auto b = s.find_first_not_of(" \t");
    if (b == std::string::npos) {
        s.clear();
        return;
    }
    const auto e = s.find_last_not_of(" \t\r");
    s = s.substr(b, e - b + 1);
}

} // namespace

bool parse(const std::string &path, std::vector<Section> &out)
{
    std::ifstream file(path);
    if (!file)
        return false;
    // The implicit first section (content before any '[') carries file-level
    // keys; dropped if it stays empty - legacy files start with comments only.
    out.clear();
    out.emplace_back();
    std::string line;
    while (getline(file, line)) {
        trim(line);
        if (line.empty() || line.front() == '#')
            continue;
        if (line.front() == '[') {
            const auto close = line.find(']');
            out.emplace_back();
            out.back().header = (close == std::string::npos)
                ? line.substr(1)
                : line.substr(1, close - 1);
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos)
            continue; // not a key/value line - format layer skips, never fails
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        trim(key);
        trim(value);
        if (!key.empty())
            out.back().params[key] = value;
    }
    if (out.front().params.empty() && out.front().header.empty())
        out.erase(out.begin());
    return true;
}

bool write(const std::string &path, const std::vector<Section> &sections,
           const std::vector<std::string> &banner)
{
    // Sibling temp in the target's own directory (INTENT §11.52(a): rename
    // atomicity is same-filesystem only - never /tmp).
    const std::string tmp = path + ".tmp";
    {
        std::ofstream file(tmp, std::ios::trunc);
        if (!file) {
            cLog::get()->write("ModularSystemFormat: can't create temporary file '" + tmp
                + "' - target '" + path + "' left untouched. Check that the directory exists and is writable.",
                LOG_TYPE::L_ERROR);
            return false;
        }
        for (const auto &line : banner)
            file << "# " << line << '\n';
        for (const auto &section : sections) {
            file << "\n[" << section.header << "]\n";
            for (const auto &kv : section.params)
                file << kv.first << " = " << kv.second << '\n';
        }
        file.flush();
        if (!file) {
            file.close();
            std::remove(tmp.c_str()); // discard the temp, NEVER the original
            cLog::get()->write("ModularSystemFormat: write to '" + tmp
                + "' failed (disk full?) - temporary discarded, target '" + path + "' left untouched.",
                LOG_TYPE::L_ERROR);
            return false;
        }
    }
    if (std::rename(tmp.c_str(), path.c_str()) != 0) {
        std::remove(tmp.c_str());
        cLog::get()->write("ModularSystemFormat: can't rename '" + tmp + "' over '" + path
            + "' - temporary discarded, target left untouched.", LOG_TYPE::L_ERROR);
        return false;
    }
    return true;
}

} // namespace ModularSystemFormat
