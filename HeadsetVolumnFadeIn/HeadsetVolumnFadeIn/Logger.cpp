#include "pch.h"
#include "Logger.h"
#include <iostream>
#include <tchar.h>
using namespace std;


Logger& Logger::CreateInstance()
{
    static Logger logger;
    return logger;
}

void Logger::Log(LOGGER_LEVEL level, const std::string& msg) const
{
    if (level >= logLevel) {
        cout << "[" + std::string(__FUNCTION__) + "]" + msg << endl;
    }
}

void Logger::Log(LOGGER_LEVEL level, const std::wstring& msg) const
{
    if (level >= logLevel) {
        wcout << L"[" + std::wstring(_T(__FUNCTION__)) + L"]" + msg << endl;
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
