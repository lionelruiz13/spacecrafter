/* F74 reproduction stub -- NOT engine code.
 * Mirrors src/tools/log.hpp's SURFACE only (enum member names and order,
 * write() signature) so that the real fontFactory.cpp compiles unchanged.
 * The sink is stdout, so every log line the real code emits shows in the trace.
 */
#ifndef _F74_STUB_LOG_HPP_
#define _F74_STUB_LOG_HPP_

#include <string>
#include <cstdio>

// [observed: src/tools/log.hpp:40-46]
enum class LOG_TYPE : char {
    L_WARNING,
    L_ERROR,
    L_DEBUG,
    L_INFO,
    L_OTHER
};

// [observed: src/tools/log.hpp:52-58]
enum class LOG_FILE : char {
    INTERNAL,
    SCRIPT,
    SHADER,
    TCP,
    VULKAN
};

class cLog {
public:
    static cLog *get() {
        static cLog instance;
        return &instance;
    }
    // [observed: src/tools/log.hpp:82]
    void write(const std::string& texte, const LOG_TYPE& type = LOG_TYPE::L_INFO,
               const LOG_FILE& fichier = LOG_FILE::INTERNAL) {
        (void)fichier;
        const char *tag = "INFO";
        switch (type) {
            case LOG_TYPE::L_WARNING: tag = "WARNING"; break;
            case LOG_TYPE::L_ERROR:   tag = "ERROR";   break;
            case LOG_TYPE::L_DEBUG:   tag = "DEBUG";   break;
            case LOG_TYPE::L_INFO:    tag = "INFO";    break;
            case LOG_TYPE::L_OTHER:   tag = "OTHER";   break;
        }
        printf("[log %-7s] %s\n", tag, texte.c_str());
        fflush(stdout);
    }
};

#endif
