#include "pch.h"
#include "Logger.h"
#include <iostream>
using namespace std;


Logger& Logger::CreateInstance()
{
    static Logger logger;
    return logger;
}

void Logger::Log(LOGGER_LEVEL level, const std::string& msg) const
{
    if (level >= logLevel) {
        cout << msg << endl;
    }
}

void Logger::SetLogLevel(LOGGER_LEVEL level)
{
    logLevel = level;
}

Logger::Logger()
{
}


Logger::~Logger()
{
}
