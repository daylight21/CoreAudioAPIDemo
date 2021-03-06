#pragma once
#include <string>

enum LOGGER_LEVEL {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_ERROR,
};

#define LOG_DEBUG(msg) Logger::CreateInstance().Log(LOGGER_LEVEL::LOG_LEVEL_DEBUG, msg)
#define LOG_INFO(msg) Logger::CreateInstance().Log(LOGGER_LEVEL::LOG_LEVEL_INFO, msg)
#define LOG_WARNING(msg) Logger::CreateInstance().Log(LOGGER_LEVEL::LOG_LEVEL_WARNING, msg)
#define LOG_ERROR(msg) Logger::CreateInstance().Log(LOGGER_LEVEL::LOG_LEVEL_ERROR, msg)

class Logger
{
public:
    static Logger& CreateInstance();
    void Log(LOGGER_LEVEL level, const std::string& msg) const;
    void Log(LOGGER_LEVEL level, const std::wstring& msg) const;
    void SetLogLevel(LOGGER_LEVEL level);
private:
    Logger();
    ~Logger();

    LOGGER_LEVEL logLevel{ LOG_LEVEL_DEBUG };
};

