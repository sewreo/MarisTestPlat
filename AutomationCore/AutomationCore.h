#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <map>
#include <stdexcept>
#include "ITestDataManager.h"
#include "IPluginManager.h"
#include "TestEngine.h"
#include "Logger.h"
#include "ReportGenerator.h"

// 前向声明
struct TestCase;
struct TestExecutionResult;

/**
 * @brief 报告格式枚举
 */



/**
 * @brief 自动化测试平台核心框架
 * 整合插件管理、测试执行、数据管理和日志系统的核心类
 */
class AutomationCore
{
public:
    /**
     * @brief 构造函数
     */
    AutomationCore();

    /**
     * @brief 析构函数
     */
    ~AutomationCore();

    /**
     * @brief 初始化核心框架
     * @param pluginDirectory 插件目录路径
     * @return 初始化成功返回true，否则返回false
     */
    bool initialize(const std::string &pluginDirectory);

    /**
     * @brief 反初始化核心框架
     */
    void uninitialize();

    /**
     * @brief 获取日志器
     * @return 日志器接口指针
     */
   // ILogger *getLogger() const;

    /**
     * @brief 设置日志级别
     * @param level 日志级别
     */
    void setLogLevel(LogLevel level);

    /**
     * @brief 启用/禁用详细日志
     * @param verbose true启用，false禁用
     */
    void setVerboseLogging(bool verbose);

    /**
     * @brief 获取数据管理器
     * @return 数据管理器接口指针
     */
    ITestDataManager *getDataManager() const;

    /**
     * @brief 获取插件信息（名称到版本的映射）
     * @return 插件信息映射
     */
    std::map<std::string, std::string> getPluginInfos() const;

    /**
     * @brief 获取指定插件支持的动作
     * @param pluginName 插件名称
     * @return 动作名称列表
     */
    std::vector<std::string> getPluginActions(const std::string &pluginName) const;

    /**
     * @brief 检查插件是否可用
     * @param pluginName 插件名称
     * @return 可用返回true，否则返回false
     */
    bool isPluginAvailable(const std::string &pluginName) const;

    /**
     * @brief 执行单个测试用例
     * @param testCase 测试用例
     * @return 执行结果
     */
    TestExecutionResult executeTestCase(const TestCase &testCase);

    /**
     * @brief 并行执行多个测试用例
     * @param testCases 测试用例列表
     * @param threadCount 线程数量，0表示使用默认值
     * @return 执行结果列表
     */
    std::vector<TestExecutionResult> executeTestCasesParallel(
        const std::vector<TestCase> &testCases,
        size_t threadCount = 0);

    /**
     * @brief 保存测试用例到文件
     * @param testCases 测试用例列表
     * @param filePath 文件路径
     * @return 保存成功返回true，否则返回false
     */
    bool saveTestCases(const std::vector<TestCase> &testCases, const std::string &filePath);

    /**
     * @brief 从文件加载测试用例
     * @param filePath 文件路径
     * @return 加载的测试用例列表
     */
    std::vector<TestCase> loadTestCases(const std::string &filePath);

    /**
     * @brief 解析文本中的数据引用
     * @param text 包含数据引用的文本
     * @return 替换了数据引用的文本
     */
    std::string resolveDataReferences(const std::string &text);

    /**
     * @brief 生成单个测试用例的报告
     * @param result 测试执行结果
     * @param format 报告格式
     * @return 报告内容
     */
    std::string generateTestCaseReport(const TestExecutionResult &result, ReportFormat format);

    /**
     * @brief 生成多个测试用例的汇总报告
     * @param results 测试执行结果列表
     * @param format 报告格式
     * @param title 报告标题
     * @return 报告内容
     */
    std::string generateReport(const std::vector<TestExecutionResult> &results,
                               ReportFormat format,
                               const std::string &title = "Test Automation Report");

    /**
     * @brief 保存报告到文件
     * @param reportContent 报告内容
     * @param filePath 文件路径
     * @return 保存成功返回true，否则返回false
     */
    bool saveReport(const std::string &reportContent, const std::string &filePath);

    /**
     * @brief 获取执行日志
     * @return 日志内容
     */
    std::string getExecutionLog() const;

    /**
     * @brief 清除执行日志
     */
    void clearExecutionLog();

private:
    // 禁用拷贝构造和赋值操作
    AutomationCore(const AutomationCore &) = delete;
    AutomationCore &operator=(const AutomationCore &) = delete;



    // 核心组件
    std::unique_ptr<IPluginManager> m_pluginManager; // 插件管理器
    std::unique_ptr<ITestDataManager> m_dataManager; // 数据管理器
    std::unique_ptr<TestEngine> m_testEngine;        // 测试引擎
    std::unique_ptr<ILogger> m_memoryLogger;         // 内存日志器（用于获取执行日志）

    bool m_initialized; // 初始化状态标志
};

#ifdef AUTOMATIONCORE_EXPORTS
#define AUTOMATIONCORE_API __declspec(dllexport)
#else
#define AUTOMATIONCORE_API __declspec(dllimport)
#endif

// C风格接口，供外部模块（如Qt UI）调用
extern "C" AUTOMATIONCORE_API AutomationCore *createAutomationCore();
extern "C" AUTOMATIONCORE_API void destroyAutomationCore(AutomationCore *core);
extern "C" AUTOMATIONCORE_API bool initializeCore(AutomationCore *core, const char *pluginDirectory);
extern "C" AUTOMATIONCORE_API void uninitializeCore(AutomationCore *core);
