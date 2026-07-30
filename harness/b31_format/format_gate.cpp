// B31 slice 1 (INTENT §11.119) - the instrument that drives the REAL write-back
// layer: it compiles `src/experimentalModule/ModularSystemFormat.cpp` and
// `src/tools/log.cpp` from the product tree, nothing is reimplemented here.
// A Python reimplementation of the parser would test a copy; this tests the
// code the app runs.
//
// WHAT IT DOES: parse a file, apply the operations named on the command line
// (the loader's role - only a loader knows a datum is wrong), write the result.
// With `dump` it prints what the parse SAW, as hex so no encoding can lie about
// an ISO-8859 byte.
//
//   format_gate <in> <out|-> [op ...]
//     set|<section>|<key>|<value>          give a key a value
//     remove|<section>|<key>|<reason>      retire a key (commented out, not deleted)
//     annotate|<section>|<key>|<reason>|<text>   what a loader diagnosed
//     dump                                 print the parse to stdout as JSON
//   <section> is a header text (first match) or '#<n>', the 0-based index.
//   '\n' inside a reason or a text becomes a real newline.
//
// Exit: 0 = every operation applied, 2 = an operation was refused (the layer
// said no - which is a legitimate answer the gate asserts on), 1 = usage/IO.

#include "experimentalModule/ModularSystemFormat.hpp"
#include "tools/log.hpp"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

namespace {

std::vector<std::string> split(const std::string &text, char sep)
{
    std::vector<std::string> out;
    std::size_t pos = 0;
    for (;;) {
        const auto at = text.find(sep, pos);
        if (at == std::string::npos) {
            out.push_back(text.substr(pos));
            return out;
        }
        out.push_back(text.substr(pos, at - pos));
        pos = at + 1;
    }
}

std::string unescape(const std::string &text)
{
    std::string out;
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\\' && i + 1 < text.size() && text[i + 1] == 'n') {
            out += '\n';
            ++i;
        } else {
            out += text[i];
        }
    }
    return out;
}

std::string hex(const std::string &bytes)
{
    static const char *DIGITS = "0123456789abcdef";
    std::string out;
    for (unsigned char c : bytes) {
        out += DIGITS[c >> 4];
        out += DIGITS[c & 0xf];
    }
    return out;
}

ModularSystemFormat::Section *locate(std::vector<ModularSystemFormat::Section> &sections,
                                     const std::string &which)
{
    if (!which.empty() && which[0] == '#') {
        const std::size_t at = std::stoul(which.substr(1));
        return (at < sections.size()) ? &sections[at] : nullptr;
    }
    for (auto &section : sections) {
        if (section.getHeader() == which)
            return &section;
    }
    return nullptr;
}

} // namespace

int main(int argc, char *argv[])
{
    if (argc < 3) {
        std::cerr << "usage: format_gate <in> <out|-> [op ...]\n";
        return 1;
    }
    // The diagnostics go to the console and nowhere else: cLog::write reaches
    // for its INTERNAL file unconditionally (log.cpp:150-156) and no log file is
    // open outside the app, so this is also what keeps a refusal from aborting
    // the tool instead of reporting itself.
    cLog::get()->setDebug(true);
    cLog::get()->setWriteLog(false);
    const std::string in = argv[1], out = argv[2];
    std::vector<ModularSystemFormat::Section> sections;
    if (!ModularSystemFormat::parse(in, sections)) {
        std::cerr << "format_gate: cannot open '" << in << "'\n";
        return 1;
    }
    bool refused = false;
    bool wantDump = false;
    for (int i = 3; i < argc; ++i) {
        const std::vector<std::string> op = split(argv[i], '|');
        if (op[0] == "dump") {
            wantDump = true;
            continue;
        }
        if (op.size() < 4) {
            std::cerr << "format_gate: malformed op '" << argv[i] << "'\n";
            return 1;
        }
        ModularSystemFormat::Section *section = locate(sections, op[1]);
        if (!section) {
            std::cerr << "format_gate: no section '" << op[1] << "'\n";
            return 1;
        }
        if (op[0] == "set") {
            if (!section->set(op[2], unescape(op[3])))
                refused = true;
        } else if (op[0] == "remove") {
            section->remove(op[2], unescape(op[3]));
        } else if (op[0] == "annotate") {
            if (op.size() < 5) {
                std::cerr << "format_gate: annotate needs a text\n";
                return 1;
            }
            section->annotate(op[2], op[3], unescape(op[4]));
        } else {
            std::cerr << "format_gate: unknown op '" << op[0] << "'\n";
            return 1;
        }
    }
    if (wantDump) {
        std::cout << "{\"sections\":[";
        for (std::size_t s = 0; s < sections.size(); ++s) {
            const auto &section = sections[s];
            std::cout << (s ? ",{" : "{") << "\"header\":\"" << hex(section.getHeader())
                      << "\",\"preamble\":" << (section.isPreamble() ? "true" : "false")
                      << ",\"params\":{";
            bool first = true;
            for (const auto &kv : section.params()) {
                std::cout << (first ? "" : ",") << '"' << hex(kv.first) << "\":\""
                          << hex(kv.second) << '"';
                first = false;
            }
            std::cout << "}}";
        }
        std::cout << "]}\n";
    }
    if (out != "-") {
        if (!ModularSystemFormat::write(out, sections, {})) {
            std::cerr << "format_gate: write to '" << out << "' failed\n";
            return 1;
        }
    }
    return refused ? 2 : 0;
}
