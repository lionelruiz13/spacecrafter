/* F74 reproduction stub -- NOT engine code.
 * Mirrors the surface of src/tools/s_font.hpp that fontFactory.cpp uses:
 * the (float, string) constructor, rebuild(), the non-virtual inline
 * getFontSize() [observed: s_font.hpp:101-103] and the static initBaseFont().
 * The virtual destructor is kept because the real class has one
 * [observed: s_font.hpp:78] -- it puts a vtable pointer at offset 0, which is
 * what the real layout does too.
 * No font is loaded: every call prints, so the driver's trace IS the list of
 * effects the real code would have produced.
 */
#ifndef _F74_STUB_S_FONT_HPP_
#define _F74_STUB_S_FONT_HPP_

#include <string>
#include <cstdio>
#include <cmath>   // the real s_font.hpp pulls it in transitively; fontFactory.cpp
                   // uses sqrtf/round without including it itself

class s_font {
public:
    s_font(float size_i, const std::string& ttfFileName)
        : fontSize(size_i), fontName(ttfFileName) {
        printf("[s_font  ctor   ] size=%g name=%s\n", (double)size_i, ttfFileName.c_str());
        fflush(stdout);
    }
    virtual ~s_font() {}

    void rebuild(float size_i, const std::string& ttfFileName) {
        printf("[s_font  rebuild] size=%g name=%s\n", (double)size_i, ttfFileName.c_str());
        fflush(stdout);
        fontSize = size_i;
        fontName = ttfFileName;
    }

    static void initBaseFont(const std::string& ttfFileName) {
        printf("[s_font  base   ] %s\n", ttfFileName.c_str());
        fflush(stdout);
    }

    float getFontSize() {
        return fontSize;
    }

protected:
    float fontSize;
    std::string fontName;
};

#endif
