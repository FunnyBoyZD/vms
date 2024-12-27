#ifndef _VMS_CORE_LOGGER_H_
#define _VMS_CORE_LOGGER_H_

#include <mutex>
#include <string>
#include <sstream>

namespace Vms { namespace Core
{
    enum LogLevel
    {
        LogLevelDEBUG = 0,
        LogLevelINFO = 1,
        LogLevelWARN = 2,
        LogLevelERROR = 3,
        LogLevelOFF = 4
    };

    class Logger
    {
    public:
        Logger() = default;
        ~Logger() = default;

        inline LogLevel level() const { return level_; }
        inline void setLevel(LogLevel logLevel) { level_ = logLevel; }

        inline bool isVerbose() const { return verbose_; }
        inline void setVerbose(bool value) { verbose_ = value; }

        inline bool isEnabledFor(LogLevel logLevel) const { return logLevel >= level_; }

        void log(LogLevel logLevel, const char* facility, const std::string& message,
            const char* file, int line);

    private:
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        std::mutex mtx_;
        LogLevel level_ = LogLevelDEBUG;
        bool verbose_ = false;
    };

    extern Logger logger;
} }

#define VMS_LOG_IMPL(facility, logEvent, logLevel) \
    do { \
        if (Vms::Core::logger.isEnabledFor(Vms::Core::LogLevel##logLevel)) { \
            std::ostringstream __os; \
            __os << logEvent; \
            Vms::Core::logger.log(Vms::Core::LogLevel##logLevel, facility, __os.str(), \
                __FILE__, __LINE__); \
        } \
    } while (0)

#define VMS_LOG_DEBUG(facility, logEvent) VMS_LOG_IMPL(facility, logEvent, DEBUG)
#define VMS_LOG_INFO(facility, logEvent) VMS_LOG_IMPL(facility, logEvent, INFO)
#define VMS_LOG_WARN(facility, logEvent) VMS_LOG_IMPL(facility, logEvent, WARN)
#define VMS_LOG_ERROR(facility, logEvent) VMS_LOG_IMPL(facility, logEvent, ERROR)

#endif
