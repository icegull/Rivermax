#pragma once
#define FMT_HEADER_ONLY
#define FMT_CONSTEVAL
#include <string_view>
#include <tchar.h>
#include "Lib.Logger/include/spdlog/fmt/bundled/xchar.h"

enum class LOGLEVEL
{
	Debug = 10000,
	Info = 20000,
	Warn = 30000,
	Error = 40000,
	Fatal = 50000,
};

//always use InitializeLogger as the first thing in main();
void InitializeLogger();

//ensures that logger shuts down before the execution leaves the main() function
void ShutdownLogger();

bool isDebugLogEnabled();

class LoggerInitializer
{
public:
	LoggerInitializer() { InitializeLogger(); }
	~LoggerInitializer() { ShutdownLogger(); }

	LoggerInitializer(LoggerInitializer const&) = delete;
	LoggerInitializer(LoggerInitializer&&) = delete;
	LoggerInitializer& operator =(LoggerInitializer const&) = delete;
	LoggerInitializer& operator =(LoggerInitializer&&) = delete;
};

void WriteLogA(std::wstring_view logPath, LOGLEVEL logLevel, const char* fmt, ...);
void WriteLogW(std::wstring_view, LOGLEVEL logLevel, const wchar_t* fmt, ...);

void WriteLog(std::wstring_view logPath, LOGLEVEL logLevel, std::string_view fmt);
void WriteLog(std::wstring_view logPath, LOGLEVEL logLevel, std::wstring_view fmt);

template <typename T, typename... Args>
void WriteLog(std::wstring_view logPath, LOGLEVEL logLevel, const T* fmt, Args&& ...args) { WriteLog(logPath, logLevel, fmt::format(fmt, std::forward<Args>(args)...)); }

template <typename T, typename... Args>
void info_log(std::wstring_view logPath, const T* fmt, Args&& ...args) { WriteLog(logPath, LOGLEVEL::Info, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void info_log(std::wstring_view logPath, const std::string_view fmt) { WriteLog(logPath, LOGLEVEL::Info, fmt); }
inline void info_log(std::wstring_view logPath, const std::wstring_view fmt) { WriteLog(logPath, LOGLEVEL::Info, fmt); }

template <typename T, typename... Args>
void warn_log(std::wstring_view logPath, const T* fmt, Args&& ...args) { WriteLog(logPath, LOGLEVEL::Warn, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void warn_log(std::wstring_view logPath, const std::string_view fmt) { WriteLog(logPath, LOGLEVEL::Warn, fmt); }
inline void warn_log(std::wstring_view logPath, const std::wstring_view fmt) { WriteLog(logPath, LOGLEVEL::Warn, fmt); }

template <typename T, typename... Args>
void error_log(std::wstring_view logPath, const T* fmt, Args&& ...args) { WriteLog(logPath, LOGLEVEL::Error, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void error_log(std::wstring_view logPath, const std::string_view fmt) { WriteLog(logPath, LOGLEVEL::Error, fmt); }
inline void error_log(std::wstring_view logPath, const std::wstring_view fmt) { WriteLog(logPath, LOGLEVEL::Error, fmt); }

template <typename T, typename... Args>
void fatal_log(std::wstring_view logPath, const T* fmt, Args&& ...args) { WriteLog(logPath, LOGLEVEL::Fatal, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void fatal_log(std::wstring_view logPath, const std::string_view fmt) { WriteLog(logPath, LOGLEVEL::Fatal, fmt); }
inline void fatal_log(std::wstring_view logPath, const std::wstring_view fmt) { WriteLog(logPath, LOGLEVEL::Fatal, fmt); }

template <typename T, typename... Args>
void debug_log(std::wstring_view logPath, const T* fmt, Args&& ...args) { if (isDebugLogEnabled()) WriteLog(logPath, LOGLEVEL::Debug, fmt::format(fmt, std::forward<Args>(args)...)); }

inline void debug_log(std::wstring_view logPath, const std::string_view fmt) { if (isDebugLogEnabled()) WriteLog(logPath, LOGLEVEL::Debug, fmt); }
inline void debug_log(std::wstring_view logPath, const std::wstring_view fmt) { if (isDebugLogEnabled()) WriteLog(logPath, LOGLEVEL::Debug, fmt); }

#define LOG_currentThreadID {SetThreadDescription(GetCurrentThread(), __FUNCTIONW__);WriteLog(L"C:\\Logs\\frame2TCP\\ThreadInfo.log",LOGLEVEL::Info,"{0} @Line:{1:5d} file:{2}",__FUNCTION__,__LINE__,__FILE__);}
