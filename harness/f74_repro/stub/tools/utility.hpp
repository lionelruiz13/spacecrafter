/* F74 reproduction stub -- NOT engine code.
 * strToDouble is copied BODY-EXACT from src/tools/utility.cpp:399-406, because
 * updateAllFont feeds it std::to_string(size/fontFactor) and the conversion is
 * part of the effect under comparison.
 */
#ifndef _F74_STUB_UTILITY_HPP_
#define _F74_STUB_UTILITY_HPP_

#include <string>

class Utility {
public:
    static double strToDouble(const std::string& str, double default_value = 0) {
        try {
            return std::stod(str);
        } catch (...) {
            return default_value;
        }
    }
};

#endif
