/* F74 reproduction stub -- NOT engine code.
 * Only getUserFontDir() is reached from fontFactory.cpp.
 */
#ifndef _F74_STUB_APP_SETTINGS_HPP_
#define _F74_STUB_APP_SETTINGS_HPP_

#include <string>

class AppSettings {
public:
    static AppSettings *Instance() {
        static AppSettings instance;
        return &instance;
    }
    const std::string getUserFontDir() const {
        return std::string("/home/claude/.spacecrafter/fonts/");
    }
};

#endif
