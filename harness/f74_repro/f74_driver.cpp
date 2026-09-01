/* F74 reproduction driver -- NOT engine code.
 *
 * Replays, against the REAL src/appModule/fontFactory.{hpp,cpp}, the two engine
 * sequences that bracket the SS.119 dereference:
 *
 *   1. App::firstInit   [observed: src/appModule/app.cpp:701-705]
 *        fontFactory->init(std::min(width,height), conf)
 *        fontFactory->initMediaFont(media.get())
 *        fontFactory->buildAllFont()
 *        fontFactory->registerFont(CLASSEFONT::CLASS_UI)
 *
 *   2. the 'zh' branch of Core::setSkyLanguage, which is what
 *      `set sky_locale zh_CN` reaches  [observed: src/coreModule/core.cpp:1589-1590]
 *        fontFactory->updateAllFont("/home/planetarium/.spacecrafter/fonts/HanWangHeiHeavy.ttf")
 *
 * min(width,height) = 1024 and the [font] values come from this host's live
 * config.ini (see stub/tools/init_parser.hpp), so fontFactor = sqrt(1024/1024) = 1.
 *
 * Everything printed between the two banners is an EFFECT the real code
 * produced; the trace is what the pre-fix and post-fix runs are compared on.
 */
#include <cstdio>
#include <cstddef>
#include <list>
#include <string>

#include "tools/s_font.hpp"   // FontContener holds unique_ptr<s_font>: the
                              // deleter needs the complete type here
#include "appModule/fontFactory.hpp"
#include "tools/init_parser.hpp"
#include "mediaModule/media.hpp"

int main()
{
    {
        // Layout probe: where does a past-the-end `it2->fontPtr` land?
        // (meaningful only without -D_GLIBCXX_DEBUG, which changes std::list)
        FontContener fc(CLASSEFONT::CLASS_MENU, 1.f, "probe");
        printf("[layout ] sizeof(std::list<FontContener>)=%zu  "
               "offsetof(FontContener,fontPtr)=%zu\n",
               sizeof(std::list<FontContener>),
               (size_t)((char *)&fc.fontPtr - (char *)&fc));
        fflush(stdout);
    }

    InitParser conf;
    Media media;
    FontFactory fontFactory;

    printf("[driver ] --- App::firstInit (app.cpp:701-705) ---\n");
    fflush(stdout);
    fontFactory.init(1024, conf);
    fontFactory.initMediaFont(&media);
    fontFactory.buildAllFont();
    fontFactory.registerFont(CLASSEFONT::CLASS_UI);

    printf("[driver ] --- set sky_locale zh_CN -> Core::setSkyLanguage "
           "-> core.cpp:1590 ---\n");
    fflush(stdout);
    fontFactory.updateAllFont("/home/planetarium/.spacecrafter/fonts/HanWangHeiHeavy.ttf");

    printf("[driver ] --- updateAllFont RETURNED, no fault ---\n");
    fflush(stdout);
    return 0;
}
