/* F74 reproduction stub -- NOT engine code.
 * Serves the [font] section of the LIVE config
 * (~/.spacecrafter/config.ini, read 2026-09-01, lines 104-127), so the sizes
 * and names in the trace are the ones this host's engine would use.
 * Values are hardcoded (not parsed) so the reproduction stays reproducible
 * from git alone; the key strings are the real macros' strings
 * [observed: src/mainModule/define_key.hpp:151-173].
 */
#ifndef _F74_STUB_INIT_PARSER_HPP_
#define _F74_STUB_INIT_PARSER_HPP_

#include <string>
#include <map>
#include <cstdlib>

class InitParser {
public:
    std::string getStr(const std::string& section, const std::string& key) const {
        (void)section;
        const std::map<std::string, std::string> v = {
            {"font_general_name",        "DejaVuSansMono.ttf"},
            {"font_menu_name",           "DejaVuSansMono-Bold.ttf"},
            {"font_planet_name",         "DejaVuSans-Bold.ttf"},
            {"font_constellation_name",  "DejaVuSans-Bold.ttf"},
            {"font_display_name",        "DejaVuSans-Bold.ttf"},
            {"font_cardinalpoints_name", "DejaVuSans-Bold.ttf"},
            {"font_grid_name",           "DejaVuSans-Bold.ttf"},
            {"font_lines_name",          "DejaVuSans.ttf"},
            {"font_hip_stars_name",      "DejaVuSans-Bold.ttf"},
            {"font_nebulas_name",        "DejaVuSans-Bold.ttf"},
            {"font_text_name",           "DejaVuSans-Bold.ttf"},
        };
        auto it = v.find(key);
        return (it == v.end()) ? std::string("") : it->second;
    }
    double getDouble(const std::string& section, const std::string& key) const {
        (void)section;
        const std::map<std::string, double> v = {
            {"font_resolution_size",     1024},
            {"font_menutui_size",          18},
            {"font_planet_size",           14},
            {"font_constellation_size",    12},
            {"font_display_size",          12},
            {"font_cardinalpoints_size",   24},
            {"font_grid_size",             12},
            {"font_line_size",             12},
            {"font_hip_stars_size",        12},
            {"font_nebulas_size",          12},
            {"font_text_size",             16},
        };
        auto it = v.find(key);
        return (it == v.end()) ? 0.0 : it->second;
    }
    int getInt(const std::string& section, const std::string& key) const {
        return (int)getDouble(section, key);
    }
};

#endif
