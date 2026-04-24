#ifndef LOG_H
#define LOG_H

#include <cstdio>
#include <format>
#include <mutex>
#include <source_location>
#include <string_view>

enum class LogLevel { Debug, Info, Warn, Error, Fatal };

template <> struct std::formatter<LogLevel> : std::formatter<std::string_view> {
    auto format(LogLevel lvl, std::format_context& ctx) const {
        std::string_view name = "?";
        switch (lvl) {
        case LogLevel::Debug:
            name = "DEBUG";
            break;
        case LogLevel::Info:
            name = "INFO";
            break;
        case LogLevel::Warn:
            name = "WARN";
            break;
        case LogLevel::Error:
            name = "ERROR";
            break;
        case LogLevel::Fatal:
            name = "FATAL";
            break;
        }
        return std::formatter<std::string_view>::format(name, ctx);
    }
};

class logger {
  public:
    static logger& instance();

    void write(LogLevel lvl, std::source_location loc, std::string_view msg);

    logger(const logger&) = delete;
    logger& operator=(const logger&) = delete;

  private:
    logger();
    ~logger();

    std::FILE* file = nullptr;
    std::mutex mutex;
};

#define LOG_INFO(fmt, ...)                                                                         \
    logger::instance().write(LogLevel::Info, std::source_location::current(),                      \
                             std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
#define LOG_WARN(fmt, ...)                                                                         \
    logger::instance().write(LogLevel::Warn, std::source_location::current(),                      \
                             std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
#define LOG_ERROR(fmt, ...)                                                                        \
    logger::instance().write(LogLevel::Error, std::source_location::current(),                     \
                             std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
#define LOG_FATAL(fmt, ...)                                                                        \
    logger::instance().write(LogLevel::Fatal, std::source_location::current(),                     \
                             std::format(fmt __VA_OPT__(, ) __VA_ARGS__))

#ifdef DEBUG
#define LOG_DEBUG(fmt, ...)                                                                        \
    logger::instance().write(LogLevel::Debug, std::source_location::current(),                     \
                             std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#endif
