#include "LogWriter.h"
#include <map>
#include <shared_mutex>
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/rotating_file_sink.h"

using spdLogger = std::shared_ptr<spdlog::logger>;
using LoggerList = std::map<std::wstring, spdLogger, std::less<>>;

struct LogConfig
{
	LoggerList   loggerList;
	long         logSize = 0;
	int32_t      logNumber = 0;
	bool         asyncLog = true;
	bool         enableDebugLog = false;
	bool         instantFlush = false;
	std::wstring loggerAppName;
};

static LogConfig         logConfig;
static std::shared_mutex locker;
static constexpr int32_t LOGSIZE = 2 * 1024;

void loadLogConfig()
{
	constexpr wchar_t const* path = LR"(C:\ProgramData\SimplyLive.TV\Vibox\Backend\LogConfig.ini)";
	logConfig.logSize = GetPrivateProfileIntW(L"Log", L"size_per_file_MB", 20, path);
	logConfig.logSize *= (1024 * 1024);
	logConfig.logNumber = GetPrivateProfileIntW(L"Log", L"file_num", 5, path);
	logConfig.enableDebugLog = GetPrivateProfileIntW(L"Log", L"EnableDebug", false, path);
	logConfig.asyncLog = GetPrivateProfileIntW(L"Log", L"Async", true, path);
	logConfig.instantFlush = GetPrivateProfileIntW(L"Log", L"InstantFlush", false, path);

	wchar_t strModuleFileName[MAX_PATH];
	wchar_t strPath[MAX_PATH];
	GetModuleFileName(nullptr, strModuleFileName, MAX_PATH);
	_wsplitpath_s(strModuleFileName, nullptr, 0, nullptr, 0, strPath + 1, std::size(strPath) - 1, nullptr, 0);
	strPath[0] = '_';
	logConfig.loggerAppName = strPath;
}

inline std::string getAString(std::wstring_view fmt, ...)
{
	if (fmt.empty())
		return {};

	va_list marker;
	wchar_t szBuffer[5 * 1024];

	va_start(marker, fmt);
	_vsnwprintf_s(szBuffer, std::size(szBuffer), _TRUNCATE, fmt.data(), marker);
	va_end(marker);

	char   buffer[5 * 1024];
	size_t size = 0;
	wcstombs_s(&size, buffer, szBuffer, wcslen(szBuffer));
	return buffer;
}

spdLogger get_logger(std::wstring_view logPath)
{
	{
		//here can be Concurrency
		std::shared_lock lock(locker);
		if (const auto iter = logConfig.loggerList.find(logPath.data()); iter != logConfig.loggerList.end())
			return iter->second;
	}

	std::scoped_lock lock(locker);
	if (const auto iter = logConfig.loggerList.find(logPath.data()); iter != logConfig.loggerList.end())
		return iter->second;

	static std::once_flag flag;
	std::call_once(flag, [&]
		{
			loadLogConfig();
			if (!logConfig.instantFlush)
				spdlog::flush_every(std::chrono::seconds(5));
		});

	std::wstring newPath(logPath);
	newPath.insert(newPath.length() - 4, logConfig.loggerAppName);

	spdLogger rotating_logger;
	if (logConfig.asyncLog)
	{
		static auto tp = std::make_shared<spdlog::details::thread_pool>(8192, 1);
		auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(newPath, logConfig.logSize, logConfig.logNumber);
		rotating_logger = std::make_shared<spdlog::async_logger>(getAString(logPath), std::move(file_sink), std::move(tp), spdlog::async_overflow_policy::overrun_oldest);
		register_logger(rotating_logger);
	}
	else
		rotating_logger = spdlog::rotating_logger_mt(getAString(logPath), newPath, logConfig.logSize, logConfig.logNumber);

	//https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
	rotating_logger->set_pattern("%C/%m/%d %T.%e [%-5t][%-5!l]- %v");
	rotating_logger->set_level(logConfig.enableDebugLog ? spdlog::level::debug : spdlog::level::info);
	if (!logConfig.instantFlush)
		rotating_logger->flush_on(spdlog::level::err);

	logConfig.loggerList.emplace(logPath, rotating_logger);
	return rotating_logger;
}

template <typename T>
void internal_writer(std::wstring_view logPath, LOGLEVEL level, const T& log)
{
	if (logPath.empty())
		return;

	const spdLogger logger = get_logger(logPath);
	switch (level)
	{
	case LOGLEVEL::Debug: logger->debug(log);
		break;
	case LOGLEVEL::Warn: logger->warn(log);
		break;
	case LOGLEVEL::Error: logger->error(log);
		break;
	case LOGLEVEL::Fatal: logger->critical(log);
		break;
	case LOGLEVEL::Info:
	default: logger->info(log);
		break;
	}

	if (logConfig.instantFlush)
		logger->flush();
}

void WriteLogW(std::wstring_view logPath, LOGLEVEL logLevel, const wchar_t* fmt, ...)
{
	wchar_t szBuffer[LOGSIZE];

	va_list marker;
	va_start(marker, fmt);
	int32_t ret = _vsnwprintf_s(szBuffer, std::size(szBuffer), _TRUNCATE, fmt, marker);
	va_end(marker);

	if (ret < 0)
		return;

	internal_writer(logPath, logLevel, szBuffer);
}

void WriteLogA(std::wstring_view logPath, LOGLEVEL logLevel, const char* fmt, ...)
{
	char szBuffer[LOGSIZE / 2];

	va_list marker;
	va_start(marker, fmt);
	int32_t ret = _vsnprintf_s(szBuffer, std::size(szBuffer), _TRUNCATE, fmt, marker);
	va_end(marker);

	if (ret < 0)
		return;

	internal_writer(logPath, logLevel, szBuffer);
}

void WriteLog(std::wstring_view logPath, LOGLEVEL logLevel, std::wstring_view fmt)
{
	internal_writer(logPath, logLevel, fmt);
}

void WriteLog(std::wstring_view logPath, LOGLEVEL logLevel, std::string_view fmt)
{
	internal_writer(logPath, logLevel, fmt);
}

void InitializeLogger()
{
}

void ShutdownLogger()
{
	spdlog::shutdown();
}

bool isDebugLogEnabled()
{
	return logConfig.enableDebugLog;
}
