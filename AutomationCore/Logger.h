#pragma once

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/ringbuffer_sink.h>
#include <stdexcept>

// 日志级别与spdlog对应（不变）
enum class LogLevel
{
    TRACE = spdlog::level::trace,
    DEBUG = spdlog::level::debug,
    INFO = spdlog::level::info,
    WARNING = spdlog::level::warn,
    ERR = spdlog::level::err,
    FATAL = spdlog::level::critical
};

// 测试数据管理异常
class LogException : public std::runtime_error {
public:
    explicit LogException(const std::string& message) : std::runtime_error(message) {}
};


// 日志接口类（移除内存日志相关方法）
class ILogger
{
public:
    virtual ~ILogger() = default;

    virtual void setLogLevel(LogLevel level) = 0;
    virtual LogLevel getLogLevel() const = 0;
    virtual void clearLog() = 0;
    virtual std::string getLogContent() = 0;

    virtual void log(LogLevel level, const std::string &message) = 0;
    virtual void log(LogLevel level, const char *format, ...) = 0;

    virtual void trace(const std::string &message) = 0;
    virtual void debug(const std::string &message) = 0;
    virtual void info(const std::string &message) = 0;
    virtual void warning(const std::string &message) = 0;
    virtual void error(const std::string &message) = 0;
    virtual void fatal(const std::string &message) = 0;

    virtual void trace(const char *format, ...) = 0;
    virtual void debug(const char *format, ...) = 0;
    virtual void info(const char *format, ...) = 0;
    virtual void warning(const char *format, ...) = 0;
    virtual void error(const char *format, ...) = 0;
    virtual void fatal(const char *format, ...) = 0;
};

// 基于spdlog的日志实现类（移除memory_sink相关成员）
class SpdLogger : public ILogger
{
public:
    // 创建默认日志器（控制台 + 滚动文件）
    SpdLogger(const std::string &loggerName = "automation_core");

    // 自定义日志器（支持传入外部sink）
    SpdLogger(const std::string &loggerName,
              const std::vector<spdlog::sink_ptr> &sinks);

    // 实现ILogger接口（不变）
    void setLogLevel(LogLevel level) override;
    LogLevel getLogLevel() const override;

    void clearLog() override;
    std::string getLogContent() override;

    void log(LogLevel level, const std::string &message) override;
    void log(LogLevel level, const char *format, ...) override;

    void trace(const std::string &message) override;
    void debug(const std::string &message) override;
    void info(const std::string &message) override;
    void warning(const std::string &message) override;
    void error(const std::string &message) override;
    void fatal(const std::string &message) override;

    void trace(const char *format, ...) override;
    void debug(const char *format, ...) override;
    void info(const char *format, ...) override;
    void warning(const char *format, ...) override;
    void error(const char *format, ...) override;
    void fatal(const char *format, ...) override;

private:
    std::shared_ptr<spdlog::logger> m_logger;
    mutable std::mutex m_mutex; // 线程安全锁
    std::string m_loggerName;
    std::shared_ptr<spdlog::sinks::rotating_file_sink_mt> m_fileSink;
    // 添加ringbuffer_sink引用
    std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> m_memorySink;
    spdlog::level::level_enum toSpdLogLevel(LogLevel level) const;
    LogLevel fromSpdLogLevel(spdlog::level::level_enum level) const;
};

// 日志管理器（单例模式，不变）
class LoggerManager
{
public:
    static LoggerManager* getInstance() {
        static LoggerManager instance;
        return &instance;
    }

    LoggerManager(const LoggerManager &) = delete;
    LoggerManager &operator=(const LoggerManager &) = delete;


    void setLogger(std::unique_ptr<ILogger> logger);
    ILogger *getLogger();
    void setGlobalLogLevel(LogLevel level);

private:
    LoggerManager();
    ~LoggerManager() = default;

    std::unique_ptr<ILogger> m_logger;
    mutable std::mutex m_mutex;
};

// 日志宏定义（不变）
#define LOG_TRACE(message) \
    LoggerManager::getInstance()->getLogger()->trace(message)

// ... 其他宏定义（DEBUG/INFO/WARNING/ERROR/FATAL 及 FMT 版本）
#define LOG_DEBUG(message) \
    LoggerManager::getInstance()->getLogger()->debug(message)
#define LOG_INFO(message) \
    LoggerManager::getInstance()->getLogger()->info(message)
#define LOG_WARNING(message) \
    LoggerManager::getInstance()->getLogger()->warning(message)
#define LOG_ERROR(message) \
    LoggerManager::getInstance()->getLogger()->error(message)
#define LOG_FATAL(message) \
    LoggerManager::getInstance()->getLogger()->fatal(message)

#define LOG_TRACE_FMT(format, ...) \
    LoggerManager::getInstance()->getLogger()->trace(format, __VA_ARGS__)

#define LOG_DEBUG_FMT(format, ...) \
    LoggerManager::getInstance()->getLogger()->debug(format, __VA_ARGS__)

#define LOG_INFO_FMT(format, ...) \
    LoggerManager::getInstance()->getLogger()->info(format, __VA_ARGS__)
#define LOG_WARNING_FMT(format, ...) \
    LoggerManager::getInstance()->getLogger()->warning(format, __VA_ARGS__)
#define LOG_ERROR_FMT(format, ...) \
    LoggerManager::getInstance()->getLogger()->error(format, __VA_ARGS__)
#define LOG_FATAL_FMT(format, ...) \
    LoggerManager::getInstance()->getLogger()->fatal(format, __VA_ARGS__)