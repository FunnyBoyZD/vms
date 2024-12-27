#include "Vms/Core/Logger.h"
#include <iostream>

namespace Vms { namespace Core
{
    static const char* logLevelStr[] = {
        "[DEBUG] ",
        "[INFO ] ",
        "[WARN ] ",
        "[ERROR] "
    };

    Logger logger;

    void Logger::log(LogLevel logLevel, const char* facility, const std::string& message,
        const char* file, int line)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        switch (logLevel) {
        case LogLevelDEBUG:
        case LogLevelINFO:
        case LogLevelWARN:
            if (verbose_) {
                std::cerr << logLevelStr[logLevel] << "(" << file << ":" << line << ") " << facility << ": " << message << std::endl;
            } else {
                std::cerr << logLevelStr[logLevel] << facility << ": " << message << std::endl;
            }
            break;
        case LogLevelERROR:
        default:
            if (verbose_) {
                std::cerr << logLevelStr[logLevel] << "(" << file << ":" << line << ") " << facility << ": " << message << std::endl;
            } else {
                std::cerr << logLevelStr[logLevel] << facility << ": " << message << std::endl;
            }
            break;
        }
    }
} }
