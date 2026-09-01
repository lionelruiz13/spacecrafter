/* F74 reproduction stub -- NOT engine code.
 * The four Media entry points fontFactory.cpp calls, with the real signatures
 * [observed: src/mediaModule/media.hpp:350,354,358,362]. Each prints, so the
 * CLASS_MENU / text-font effects appear in the trace.
 */
#ifndef _F74_STUB_MEDIA_HPP_
#define _F74_STUB_MEDIA_HPP_

#include <string>
#include <cstdio>

class Media {
public:
    void setTextFont(float font_size, const std::string& font_name) {
        printf("[media   setText] size=%g name=%s\n", (double)font_size, font_name.c_str());
        fflush(stdout);
    }
    void buildTextFont() {
        printf("[media   build  ]\n");
        fflush(stdout);
    }
    void resetTextFont() {
        printf("[media   reset  ]\n");
        fflush(stdout);
    }
    void updateTextFont(double size, const std::string& fontName) {
        printf("[media   update ] size=%g name=%s\n", size, fontName.c_str());
        fflush(stdout);
    }
};

#endif
