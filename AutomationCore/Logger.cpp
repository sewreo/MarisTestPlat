#include "Logger.h"
#include <cstdarg>
#include <mutex>
#include <fstream>
#include <sstream>

// 默认日志器构造（控制台 + 滚动文件）
SpdLogger::SpdLogger(const std::string &loggerName)
{
    // 1. 控制台sink（带颜色，多线程安全）
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v"); // 带颜色的格式

    // 2. 滚动文件sink（避免单文件过大，多线程安全）
    // 配置：单个文件5MB，最多保留3个备份
    m_fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/automation_core.log",
            5 * 1024 * 1024,
            3
            );
    m_fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    // 3. 添加内存sink，存储最后1000条日志
    m_memorySink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1000);

    // 4. 组合所有sink并创建日志器
    std::vector<spdlog::sink_ptr> sinks = { console_sink, m_fileSink, m_memorySink };
    m_logger = std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end());
 
    // 默认日志级别：DEBUG（开发环境）/INFO（生产环境可调整）
    m_logger->set_level(spdlog::level::debug);
}

// 自定义sink构造（不变）
SpdLogger::SpdLogger(const std::string &loggerName, const std::vector<spdlog::sink_ptr> &sinks)
    : m_logger(std::make_shared<spdlog::logger>(loggerName, sinks.begin(), sinks.end()))
{
    m_logger->set_level(spdlog::level::debug);
}

// 日志级别设置（不变）
void SpdLogger::setLogLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logger->set_level(toSpdLogLevel(level));
}

LogLevel SpdLogger::getLogLevel() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return fromSpdLogLevel(m_logger->level());
}
void SpdLogger::clearLog()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    try
    {
        // 重新创建所有sink
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

        m_fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/automation_core.log",
            5 * 1024 * 1024,
            3);
        m_fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

        m_memorySink = std::make_shared<spdlog::sinks::ringbuffer_sink_mt>(1000);

        // 重新创建日志器
        std::vector<spdlog::sink_ptr> sinks = { console_sink, m_fileSink, m_memorySink };
        m_logger = std::make_shared<spdlog::logger>("automation_core", sinks.begin(), sinks.end());
        m_logger->set_level(spdlog::level::debug);

        m_logger->info("Log cleared by user request");
    }
    catch (const std::exception& ex)
    {
        throw LogException("Failed to clear log");
    }
}
std::string SpdLogger::getLogContent()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string logContent;

    try
    {
        // 如果有内存sink，优先从内存获取
        if (m_memorySink)
        {
            // 获取格式化后的所有日志消息
            auto logMessages = m_memorySink->last_formatted();

            // 合并所有日志消息
            std::stringstream buffer;
            for (const auto& message : logMessages)
            {
                buffer << message << "\n";
            }

            logContent = buffer.str();

            // 如果内存中有日志内容，直接返回
            if (!logContent.empty())
            {
                return logContent;
            }
        }

        // 否则从文件获取
        if (m_fileSink)
        {
            auto logFilePath = m_fileSink->filename();
            std::ifstream logFile(logFilePath);
            if (logFile.is_open())
            {
                std::stringstream buffer;
                buffer << logFile.rdbuf();
                logContent = buffer.str();
                logFile.close();
            }
            else
            {
                logContent = "Log file not found or cannot be opened: " + logFilePath;
            }
        }
        else
        {
            logContent = "No sinks available to retrieve log content";
        }
    }
    catch (const std::exception& ex)
    {
        logContent = "Failed to read log content: " + std::string(ex.what());
    }

    return logContent;
}
// 通用日志方法（不变）
void SpdLogger::log(LogLevel level, const std::string &message)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logger->log(toSpdLogLevel(level), message);
}

void SpdLogger::log(LogLevel level, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logger->log(toSpdLogLevel(level), format, args);
    va_end(args);
}

// 各级别日志实现（不变）
void SpdLogger::trace(const std::string &message)
{
    log(LogLevel::TRACE, message);
}

void SpdLogger::debug(const std::string &message)
{
    log(LogLevel::DEBUG, message);
}

void SpdLogger::info(const std::string& message)
{
    log(LogLevel::INFO, message);
}
void SpdLogger::warning(const std::string& message)
{
    log(LogLevel::WARNING, message);
}
void SpdLogger::error(const std::string& message)
{
    log(LogLevel::ERR, message);
}
void SpdLogger::fatal(const std::string& message)
{
    log(LogLevel::FATAL, message);
}


// 格式化日志实现（不变）
void SpdLogger::trace(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log(LogLevel::TRACE, format, args);
    va_end(args);
}

void SpdLogger::debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    log(LogLevel::DEBUG, format, args);
    va_end(args);
}

void SpdLogger::info(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log(LogLevel::INFO, format, args);
    va_end(args);
}
void SpdLogger::warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log(LogLevel::WARNING, format, args);
    va_end(args);
}
void SpdLogger::error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log(LogLevel::ERR, format, args);
    va_end(args);
}
void SpdLogger::fatal(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log(LogLevel::FATAL, format, args);
    va_end(args);
}


// 级别转换工具（不变）
spdlog::level::level_enum SpdLogger::toSpdLogLevel(LogLevel level) const
{
    return static_cast<spdlog::level::level_enum>(level);
}

LogLevel SpdLogger::fromSpdLogLevel(spdlog::level::level_enum level) const
{
    return static_cast<LogLevel>(level);
}

// LoggerManager实现（不变）
LoggerManager::LoggerManager()
{
    // 默认初始化SpdLogger
    setLogger(std::make_unique<SpdLogger>());
}



void LoggerManager::setLogger(std::unique_ptr<ILogger> logger)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_logger = std::move(logger);
}

ILogger *LoggerManager::getLogger()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_logger.get();
}

void LoggerManager::setGlobalLogLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logger)
    {
        m_logger->setLogLevel(level);
    }
}
