#include "AutomationCore.h"
#include "PluginManager.h"
#include "TestDataManager.h"
#include "TestEngine.h"
#include "TestCaseSerializer.h"
#include "Logger.h"
#include <stdexcept>
#include <sstream>
#include <fstream>


std::string strReportFormat[3] =
{
    "TEXT",
    "HTML",
    "XML"
};

std::string reportFormatToString(ReportFormat emfomat)
{
    return strReportFormat[static_cast<int>(emfomat)];
}




// 构造函数
AutomationCore::AutomationCore()
    : m_initialized(false)
{
    // 初始化日志器
    m_memoryLogger = std::make_unique<SpdLogger>();

    // 记录核心初始化日志
    LOG_INFO("AutomationCore instance created");
}

// 析构函数
AutomationCore::~AutomationCore()
{
    if (m_initialized)
    {
        uninitialize();
    }
    LOG_INFO("AutomationCore instance destroyed");
}

// 初始化核心框架
bool AutomationCore::initialize(const std::string &pluginDirectory)
{
    if (m_initialized)
    {
        LOG_WARNING("AutomationCore is already initialized");
        return true;
    }

    try
    {
        LOG_INFO("Initializing AutomationCore...");

        // 初始化数据管理器
        LOG_DEBUG("Initializing TestDataManager");
        m_dataManager = std::make_unique<TestDataManager>();
        if (!m_dataManager)
        {
            LOG_ERROR("Failed to create TestDataManager instance");
            return false;
        }

        // 初始化插件管理器
        LOG_DEBUG("Initializing PluginManager with plugin directory: " + pluginDirectory);
        m_pluginManager = std::make_unique<PluginManager>();
        if (!m_pluginManager)
        {
            LOG_ERROR("Failed to create PluginManager instance");
            return false;
        }

        // 加载插件
        int loadedPlugins = m_pluginManager->loadPluginsFromDirectory(pluginDirectory);
        LOG_INFO_FMT("Loaded %d plugins from directory", loadedPlugins);

        if (loadedPlugins == 0)
        {
            LOG_WARNING("No plugins loaded - some functionality may be limited");
        }

        // 初始化测试引擎
        LOG_DEBUG("Initializing TestEngine");
        m_testEngine = std::make_unique<TestEngine>(m_pluginManager.get(), m_dataManager.get());
        if (!m_testEngine)
        {
            LOG_ERROR("Failed to create TestEngine instance");
            return false;
        }

        // 设置测试引擎的日志器， 待后续添加
    //    m_testEngine->setLogger(getLogger());

        m_initialized = true;
        LOG_INFO("AutomationCore initialized successfully");
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR_FMT("Failed to initialize AutomationCore: %s", e.what());
        return false;
    }
}

// 反初始化核心框架
void AutomationCore::uninitialize()
{
    if (!m_initialized)
    {
        LOG_WARNING("AutomationCore is not initialized");
        return;
    }

    LOG_INFO("Uninitializing AutomationCore...");

    // 销毁测试引擎
    m_testEngine.reset();

    // 卸载所有插件
    if (m_pluginManager)
    {
        int pluginCount = m_pluginManager->getPlugins().size();
        m_pluginManager->unloadAllPlugins();
        LOG_INFO_FMT("Unloaded %d plugins", pluginCount);
        m_pluginManager.reset();
    }

    // 销毁数据管理器
    m_dataManager.reset();

    m_initialized = false;
    LOG_INFO("AutomationCore uninitialized successfully");
}



// 启用/禁用详细日志
void AutomationCore::setVerboseLogging(bool verbose)
{
    if (verbose)
    {
        setLogLevel(LogLevel::DEBUG);
    }
    else
    {
        setLogLevel(LogLevel::INFO);
    }
    LOG_INFO_FMT("Verbose logging %s", verbose ? "enabled" : "disabled");
}


void AutomationCore::setLogLevel(LogLevel level)
{
    if (m_memoryLogger)
    {
        m_memoryLogger->setLogLevel(level);
    }
}

// 获取数据管理器
ITestDataManager *AutomationCore::getDataManager() const
{
    if (!m_initialized)
    {
        LOG_ERROR("Cannot get TestDataManager - AutomationCore is not initialized");
        return nullptr;
    }
    return m_dataManager.get();
}

// 获取插件信息
std::map<std::string, std::string> AutomationCore::getPluginInfos() const
{
    if (!m_initialized || !m_pluginManager)
    {
        LOG_ERROR("Cannot get plugin infos - AutomationCore is not initialized");
        return {};
    }

    std::map<std::string, std::string> result;
    auto plugins = m_pluginManager->getPlugins();

    for (const auto &plugin : plugins)
    {
        result[plugin->name()] = plugin->version();
    }

    LOG_DEBUG_FMT("Retrieved info for %d plugins", result.size());
    return result;
}

// 获取指定插件支持的动作
std::vector<std::string> AutomationCore::getPluginActions(const std::string &pluginName) const
{
    if (!m_initialized || !m_pluginManager)
    {
        LOG_ERROR("Cannot get plugin actions - AutomationCore is not initialized");
        return {};
    }

    auto plugin = m_pluginManager->getPlugin(pluginName);
    if (!plugin)
    {
        LOG_WARNING_FMT("Plugin not found: %s", pluginName.c_str());
        return {};
    }

    auto actions = plugin->supportedActions();
    LOG_DEBUG_FMT("Retrieved %d actions for plugin: %s", actions.size(), pluginName.c_str());
    return actions;
}

// 检查插件是否可用
bool AutomationCore::isPluginAvailable(const std::string &pluginName) const
{
    if (!m_initialized || !m_pluginManager)
    {
        LOG_ERROR("Cannot check plugin availability - AutomationCore is not initialized");
        return false;
    }

    bool available = m_pluginManager->getPlugin(pluginName) != nullptr;
    LOG_DEBUG_FMT("Plugin %s is %s", pluginName.c_str(), available ? "available" : "not available");
    return available;
}

// 执行单个测试用例
TestExecutionResult AutomationCore::executeTestCase(const TestCase &testCase)
{
    if (!m_initialized || !m_testEngine)
    {
        LOG_ERROR("Cannot execute test case - AutomationCore is not initialized");
        return TestExecutionResult();
    }

    LOG_INFO_FMT("Starting execution of test case: %s (ID: %d)", testCase.name.c_str(), testCase.id);

    try
    {
        // 记录测试用例信息
        LOG_DEBUG_FMT("Test case '%s' has %d steps", testCase.name.c_str(), testCase.steps.size());

        // 执行测试用例
        auto result = m_testEngine->executeTestCase(testCase);

        // 记录执行结果
        if (result.overall_success)
        {
            LOG_INFO_FMT("Test case '%s' executed successfully in %lldms",
                         testCase.name.c_str(),
                         result.total_duration.count());
        }
        else
        {
            LOG_ERROR_FMT("Test case '%s' failed after %lldms",
                          testCase.name.c_str(),
                          result.total_duration.count());
        }

        return result;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR_FMT("Exception occurred while executing test case '%s': %s",
                      testCase.name.c_str(), e.what());

        // 返回失败结果
        TestExecutionResult result;
        result.case_id = testCase.id;
        result.case_name = testCase.name;
        result.overall_success = false;
        result.total_duration = std::chrono::milliseconds(0);

        StepExecutionResult stepResult;
        stepResult.step_id = 0;
        stepResult.result.success = false;
        stepResult.result.message = "Exception: " + std::string(e.what());
        stepResult.duration = std::chrono::milliseconds(0);

        result.step_results.push_back(stepResult);
        return result;
    }
}

// 并行执行多个测试用例
std::vector<TestExecutionResult> AutomationCore::executeTestCasesParallel(
    const std::vector<TestCase> &testCases,
    size_t threadCount)
{

    if (!m_initialized || !m_testEngine)
    {
        LOG_ERROR("Cannot execute test cases in parallel - AutomationCore is not initialized");
        return {};
    }

    if (testCases.empty())
    {
        LOG_WARNING("No test cases provided for parallel execution");
        return {};
    }

    // 确定线程数量
    if (threadCount == 0)
    {
        threadCount = std::thread::hardware_concurrency();
        if (threadCount == 0)
            threadCount = 2; // 保底值
    }

    LOG_INFO_FMT("Starting parallel execution of %d test cases using %d threads",
                 testCases.size(), threadCount);
    /*
    *暂时不支持，后续应该是多个testEngine并行，但是一个testengine中只能串行执行
    try
    {
        auto results = m_testEngine->executeTestCasesParallel(testCases, threadCount);

        // 统计结果
        int succeeded = 0, failed = 0;
        for (const auto &result : results)
        {
            if (result.overall_success)
                succeeded++;
            else
                failed++;
        }

        LOG_INFO_FMT("Parallel execution completed - %d succeeded, %d failed", succeeded, failed);
        return results;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR_FMT("Exception occurred during parallel execution: %s", e.what());
        return {};
    }
    */
    return {};
}

// 保存测试用例到文件
bool AutomationCore::saveTestCases(const std::vector<TestCase> &testCases, const std::string &filePath)
{
    if (testCases.empty())
    {
        LOG_WARNING("No test cases to save to file: " + filePath);
        return false;
    }

    try
    {
        TestCaseSerializer serializer;
        bool result = serializer.serializeToFile(testCases, filePath);

        if (result)
        {
            LOG_INFO_FMT("Successfully saved %d test cases to file: %s",
                         testCases.size(), filePath.c_str());
        }
        else
        {
            LOG_ERROR_FMT("Failed to save test cases to file: %s", filePath.c_str());
        }

        return result;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR_FMT("Exception while saving test cases to file: %s - %s",
                      filePath.c_str(), e.what());
        return false;
    }
}

// 从文件加载测试用例
std::vector<TestCase> AutomationCore::loadTestCases(const std::string &filePath)
{
    try
    {
        TestCaseSerializer serializer;
        auto testCases = serializer.deserializeCasesFromFile(filePath);

        LOG_INFO_FMT("Successfully loaded %d test cases from file: %s",
                     testCases.size(), filePath.c_str());
        return testCases;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR_FMT("Exception while loading test cases from file: %s - %s",
                      filePath.c_str(), e.what());
        return {};
    }
}

// 解析文本中的数据引用
std::string AutomationCore::resolveDataReferences(const std::string &text)
{
    if (!m_initialized || !m_dataManager)
    {
        LOG_ERROR("Cannot resolve data references - AutomationCore is not initialized");
        return text;
    }

    // 简单实现：查找${dataset.item}格式的引用并替换
    std::string result = text;
    size_t pos = 0;

    while ((pos = result.find("${", pos)) != std::string::npos)
    {
        size_t endPos = result.find("}", pos);
        if (endPos == std::string::npos)
            break;

        std::string ref = result.substr(pos + 2, endPos - pos - 2);
        size_t dotPos = ref.find('.');

        if (dotPos != std::string::npos)
        {
            std::string datasetName = ref.substr(0, dotPos);
            std::string itemName = ref.substr(dotPos + 1);

            LOG_DEBUG_FMT("Resolving data reference: %s -> %s.%s", ref.c_str(), datasetName.c_str(), itemName.c_str());

            // 查找数据集
            auto dataSets = m_dataManager->getAllDataSets();
            bool found = false;

            for (const auto &ds : dataSets)
            {
                if (ds->name == datasetName)
                {
                    // 查找数据项
                    auto item = m_dataManager->getDataItemByName(ds->id, itemName);
                    if (item)
                    {
                        // 替换引用
                        result.replace(pos, endPos - pos + 1, item->value);
                        pos += item->value.length();
                        found = true;
                        LOG_DEBUG_FMT("Resolved reference '%s' to value: %s", ref.c_str(), item->value.c_str());
                        break;
                    }
                }
            }

            if (!found)
            {
                LOG_WARNING_FMT("Could not resolve data reference: %s", ref.c_str());
                pos = endPos + 1;
            }
        }
        else
        {
            LOG_WARNING_FMT("Invalid data reference format: %s", ref.c_str());
            pos = endPos + 1;
        }
    }

    return result;
}

// 生成单个测试用例的报告
std::string AutomationCore::generateTestCaseReport(const TestExecutionResult &result, ReportFormat format)
{
    try
    {
        ReportGenerator generator;
        std::string report = generator.generateTestCaseReport(result, format);

        if (report.empty())
        {
            LOG_WARNING_FMT("Generated empty report for test case %d", result.case_id);
        }
        else
        {
            LOG_INFO_FMT("Generated %s report for test case %d",
                         reportFormatToString(format).c_str(),
                         result.case_id);
        }

        return report;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR_FMT("Exception while generating test case report: %s", e.what());
        return "";
    }
}

// 生成多个测试用例的汇总报告
std::string AutomationCore::generateReport(const std::vector<TestExecutionResult> &results,
                                           ReportFormat format,
                                           const std::string &title)
{
    if (results.empty())
    {
        LOG_WARNING("No test results to generate report");
        return "";
    }

    try
    {
        ReportGenerator generator;
        std::string report = generator.generateSummaryReport(results, format, title);

        LOG_INFO_FMT("Generated %s summary report with %d test results",
                     reportFormatToString(format).c_str(),
                     results.size());
        return report;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR_FMT("Exception while generating summary report: %s", e.what());
        return "";
    }
}

// 保存报告到文件
bool AutomationCore::saveReport(const std::string &reportContent, const std::string &filePath)
{
    if (reportContent.empty())
    {
        LOG_WARNING("Cannot save empty report to file: " + filePath);
        return false;
    }

    try
    {
    
        std::ofstream file(filePath);
        if (!file.is_open())
        {
            LOG_ERROR("Could not open file for writing report: " + filePath);
            return false;
        }

        file << reportContent;
        file.close();

        LOG_INFO("Report saved to file: " + filePath);
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_ERROR_FMT("Exception while saving report to file: %s - %s",
                      filePath.c_str(), e.what());
        return false;
    }
}

// 获取执行日志
std::string AutomationCore::getExecutionLog() const
{
    if (m_memoryLogger)
    {
        return m_memoryLogger->getLogContent();
    }
    return "";
}

// 清除执行日志
void AutomationCore::clearExecutionLog()
{
    if (m_memoryLogger)
    {
        m_memoryLogger->clearLog();
        LOG_DEBUG("Execution log cleared");
    }
}



// C风格接口实现
extern "C" AUTOMATIONCORE_API AutomationCore *createAutomationCore()
{
    try
    {
        return new AutomationCore();
    }
    catch (...)
    {
        return nullptr;
    }
}

extern "C" AUTOMATIONCORE_API void destroyAutomationCore(AutomationCore *core)
{
    if (core)
    {
        delete core;
    }
}

extern "C" AUTOMATIONCORE_API bool initializeCore(AutomationCore *core, const char *pluginDirectory)
{
    if (core && pluginDirectory)
    {
        return core->initialize(pluginDirectory);
    }
    return false;
}

extern "C" AUTOMATIONCORE_API void uninitializeCore(AutomationCore *core)
{
    if (core)
    {
        core->uninitialize();
    }
}
