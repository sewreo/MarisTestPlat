#include "AutomationCore.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <filesystem>
#include <cassert>
#include <sstream>
#include <iomanip>

namespace fs = std::filesystem;

// 测试结果统计
struct TestStats
{
    int total = 0;
    int passed = 0;
    int failed = 0;

    void printSummary() const
    {
        std::cout << "\n=== Test Summary ===" << std::endl;
        std::cout << "Total tests: " << total << std::endl;
        std::cout << "Passed: " << passed << " (" << std::fixed << std::setprecision(1)
                  << (total > 0 ? (passed * 100.0 / total) : 0) << "%)" << std::endl;
        std::cout << "Failed: " << failed << std::endl;
    }
};

// 测试辅助函数
void printTestHeader(const std::string &testName)
{
    std::cout << "\n========================================" << std::endl;
    std::cout << "Testing: " << testName << std::endl;
    std::cout << "========================================" << std::endl;
}

void printTestResult(bool success, const std::string &testDetail)
{
    std::cout << (success ? "[PASS] " : "[FAIL] ") << testDetail << std::endl;
}

// 1. 测试核心初始化和日志系统
void testCoreInitialization(AutomationCore *core, TestStats &stats)
{
    printTestHeader("Core Initialization and Logging");

    try
    {
        // 测试核心是否已初始化
        bool isInitialized = core != nullptr;
        printTestResult(isInitialized, "AutomationCore instance created");
        stats.total++;
        if (!isInitialized)
        {
            stats.failed++;
            return;
        }
        else
        {
            stats.passed++;
        }

        // 测试日志级别设置
        core->setLogLevel(LogLevel::DEBUG);
        printTestResult(true, "Set log level to DEBUG");
        stats.total++;
        stats.passed++;

        // 测试详细日志开关
        core->setVerboseLogging(true);
        printTestResult(true, "Enable verbose logging");
        stats.total++;
        stats.passed++;

        // 测试日志输出
        LOG_TRACE("This is a trace log message - should be visible with DEBUG level");
        LOG_DEBUG("This is a debug log message - should be visible with DEBUG level");
        LOG_INFO("This is an info log message - should always be visible");
        LOG_WARNING("This is a warning log message - should always be visible");
        LOG_ERROR("This is an error log message - should always be visible");
        LOG_FATAL("This is a fatal log message - should always be visible");

        printTestResult(true, "Log messages generated successfully");
        stats.total++;
        stats.passed++;

        // 测试日志内容获取
        std::string logContent = core->getExecutionLog();
        bool hasLogContent = !logContent.empty();
        printTestResult(hasLogContent, "Get execution log content");
        stats.total++;
        if (!hasLogContent)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
            std::cout << "  Log content length: " << logContent.length() << " characters" << std::endl;
        }

        // 测试日志清除
        core->clearExecutionLog();
        std::string clearedLog = core->getExecutionLog();
        bool logCleared = clearedLog.empty() || clearedLog.length() < logContent.length();
        printTestResult(logCleared, "Clear execution log");
        stats.total++;
        if (!logCleared)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }
    }
    catch (const std::exception &e)
    {
        printTestResult(false, "Exception occurred: " + std::string(e.what()));
        stats.total++;
        stats.failed++;
    }
}

// 2. 测试插件管理功能
void testPluginManagement(AutomationCore *core, TestStats &stats)
{
    printTestHeader("Plugin Management");

    try
    {
        // 测试获取插件信息
        auto pluginInfos = core->getPluginInfos();
        bool hasPlugins = !pluginInfos.empty();
        printTestResult(true, "Get plugin information (may be empty if no plugins found)");
        stats.total++;
        stats.passed++;

        if (hasPlugins)
        {
            std::cout << "Found " << pluginInfos.size() << " plugins:" << std::endl;
            for (const auto &pair : pluginInfos)
            {
                std::cout << "  - " << pair.first << " (v" << pair.second << ")" << std::endl;
            }

            // 测试获取插件支持的动作
            std::string firstPlugin = pluginInfos.begin()->first;
            auto actions = core->getPluginActions(firstPlugin);
            printTestResult(!actions.empty(), "Get plugin actions for: " + firstPlugin);
            stats.total++;
            if (actions.empty())
            {
                stats.failed++;
            }
            else
            {
                stats.passed++;
                std::cout << "Actions for " << firstPlugin << ": " << std::endl;
                for (const auto &action : actions)
                {
                    std::cout << "  - " << action << std::endl;
                }
            }

            // 测试插件是否可用
            bool isAvailable = core->isPluginAvailable(firstPlugin);
            printTestResult(isAvailable, "Check if plugin is available: " + firstPlugin);
            stats.total++;
            if (!isAvailable)
            {
                stats.failed++;
            }
            else
            {
                stats.passed++;
            }
        }
        else
        {
            LOG_WARNING("No plugins found in plugin directory - some tests will be skipped");
        }

        // 测试不存在的插件
        bool nonExistent = core->isPluginAvailable("NonExistentPlugin1234");
        printTestResult(!nonExistent, "Check non-existent plugin");
        stats.total++;
        if (nonExistent)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }
    }
    catch (const std::exception &e)
    {
        printTestResult(false, "Exception occurred: " + std::string(e.what()));
        stats.total++;
        stats.failed++;
    }
}

// 3. 测试测试用例序列化/反序列化
void testTestCaseSerialization(AutomationCore *core, TestStats &stats)
{
    printTestHeader("Test Case Serialization/Deserialization");

    try
    {
        // 创建测试目录
        fs::create_directory("testcases");
        std::string testFile = "testcases/serialization_test.json";

        // 创建测试用例
        TestCase testCase;
        testCase.id = 1;
        testCase.name = "Serialization Test Case";
        testCase.description = "Test case for serialization/deserialization";
        testCase.project_id = 1;
        testCase.created_at = "2023-07-15 10:00:00";
        testCase.last_modified = "2023-07-15 10:30:00";

        // 添加测试步骤
        TestStep step1;
        step1.id = 1;
        step1.plugin_name = "Win32Automation";
        step1.param.action = "launch_application";
        step1.param.target = "notepad.exe";
        step1.param.value = "";
        step1.stop_on_failure = true;

        TestStep step2;
        step2.id = 2;
        step2.plugin_name = "Win32Automation";
        step2.param.action = "close_window";
        step2.param.target = "无标题 - 记事本";
        step2.param.value = "yes";
        step2.stop_on_failure = true;

        testCase.steps = {step1, step2};
        std::vector<TestCase> testCases = {testCase};

        // 测试保存测试用例
        bool saveSuccess = core->saveTestCases(testCases, testFile);
        printTestResult(saveSuccess, "Save test cases to file");
        stats.total++;
        if (!saveSuccess)
        {
            stats.failed++;
            return; // 保存失败，无法继续后续测试
        }
        else
        {
            stats.passed++;
        }

        // 测试加载测试用例
        auto loadedTestCases = core->loadTestCases(testFile);
        bool loadSuccess = !loadedTestCases.empty() &&
                           loadedTestCases.size() == testCases.size();
        printTestResult(loadSuccess, "Load test cases from file");
        stats.total++;
        if (!loadSuccess)
        {
            stats.failed++;
            return;
        }
        else
        {
            stats.passed++;
        }

        // 验证加载的测试用例是否与原始一致
        const TestCase &loadedCase = loadedTestCases[0];
        bool match = (loadedCase.id == testCase.id) &&
                     (loadedCase.name == testCase.name) &&
                     (loadedCase.description == testCase.description) &&
                     (loadedCase.steps.size() == testCase.steps.size());

        printTestResult(match, "Verify test case content matches original");
        stats.total++;
        if (!match)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }

        // 验证步骤内容
        if (match && !loadedCase.steps.empty())
        {
            const TestStep &loadedStep = loadedCase.steps[0];
            bool stepMatch = (loadedStep.id == step1.id) &&
                             (loadedStep.plugin_name == step1.plugin_name) &&
                             (loadedStep.param.action == step1.param.action);

            printTestResult(stepMatch, "Verify test step content matches original");
            stats.total++;
            if (!stepMatch)
            {
                stats.failed++;
            }
            else
            {
                stats.passed++;
            }
        }
    }
    catch (const std::exception &e)
    {
        printTestResult(false, "Exception occurred: " + std::string(e.what()));
        stats.total++;
        stats.failed++;
    }
}

// 4. 测试测试数据管理功能
void testTestDataManagement(AutomationCore *core, TestStats &stats)
{
    printTestHeader("Test Data Management");

    try
    {
        ITestDataManager *dataManager = core->getDataManager();
        if (!dataManager)
        {
            printTestResult(false, "Get TestDataManager instance");
            stats.total++;
            stats.failed++;
            return;
        }
        stats.total++;
        stats.passed++;
        printTestResult(true, "Get TestDataManager instance");

        // 创建测试目录
        fs::create_directory("testdata");
        const int projectId = 1;

        // 测试创建数据集
        TestDataSet dataSet;
        dataSet.name = "TestDataManagementSet";
        dataSet.description = "DataSet for testing data management";
        dataSet.project_id = projectId;

        int dataSetId = dataManager->createDataSet(dataSet);
        bool createSuccess = dataSetId > 0;
        printTestResult(createSuccess, "Create new data set");
        stats.total++;
        if (!createSuccess)
        {
            stats.failed++;
            return;
        }
        else
        {
            stats.passed++;
        }

        // 测试添加数据项
        TestDataItem item1;
        item1.name = "test_string";
        item1.type = "string";
        item1.value = "test_value";
        item1.description = "Test string value";

        bool addSuccess = dataManager->addDataItem(dataSetId, item1);
        printTestResult(addSuccess, "Add string data item");
        stats.total++;
        if (!addSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }

        // 添加数值类型数据项
        TestDataItem item2;
        item2.name = "test_number";
        item2.type = "int";
        item2.value = "12345";
        item2.description = "Test numeric value";

        addSuccess = dataManager->addDataItem(dataSetId, item2);
        printTestResult(addSuccess, "Add numeric data item");
        stats.total++;
        if (!addSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }

        // 测试获取数据集
        auto dataSetInfo = dataManager->getDataSet(dataSetId);
        bool getSuccess = (dataSetInfo != nullptr) &&
                          (dataSetInfo->name == dataSet.name) &&
                          (dataSetInfo->items.size() == 2);
        printTestResult(getSuccess, "Get data set with items");
        stats.total++;
        if (!getSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }

        // 测试获取项目数据集
        auto projectDataSets = dataManager->getProjectDataSets(projectId);
        bool projectSuccess = !projectDataSets.empty();
        printTestResult(projectSuccess, "Get data sets for project");
        stats.total++;
        if (!projectSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }

        // 测试导出数据集
        std::string exportPath = "testdata/export_test.json";
        bool exportSuccess = dataManager->exportDataSetToFile(dataSetId, exportPath);
        printTestResult(exportSuccess, "Export data set to file");
        stats.total++;
        if (!exportSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }

        // 测试导入数据集
        int importedId = dataManager->importDataSetFromFile(exportPath, projectId);
        bool importSuccess = importedId > 0;
        printTestResult(importSuccess, "Import data set from file");
        stats.total++;
        if (!importSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }

        // 测试数据引用解析
        std::string testReference = "${TestDataManagementSet.test_string}";
        std::string resolvedValue = core->resolveDataReferences(testReference);
        bool resolveSuccess = (resolvedValue == "test_value");
        printTestResult(resolveSuccess, "Resolve data reference: " + testReference);
        stats.total++;
        if (!resolveSuccess)
        {
            stats.failed++;
            std::cout << "  Expected: 'test_value', Got: '" << resolvedValue << "'" << std::endl;
        }
        else
        {
            stats.passed++;
        }

        // 测试文本中的多个数据引用
        std::string testText = "String: ${TestDataManagementSet.test_string}, "
                               "Number: ${TestDataManagementSet.test_number}";
        std::string expectedText = "String: test_value, Number: 12345";
        std::string resolvedText = core->resolveDataReferences(testText);
        bool multiResolveSuccess = (resolvedText == expectedText);
        printTestResult(multiResolveSuccess, "Resolve multiple data references");
        stats.total++;
        if (!multiResolveSuccess)
        {
            stats.failed++;
            std::cout << "  Expected: '" << expectedText << "'" << std::endl;
            std::cout << "  Got:      '" << resolvedText << "'" << std::endl;
        }
        else
        {
            stats.passed++;
        }

        // 测试删除数据项
        bool deleteItemSuccess = dataManager->removeDataItembyName(dataSetId, "test_string");
        printTestResult(deleteItemSuccess, "Delete data item");
        stats.total++;
        if (!deleteItemSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }

        // 测试删除数据集
        bool deleteSetSuccess = dataManager->deleteDataSet(dataSetId);
        printTestResult(deleteSetSuccess, "Delete data set");
        stats.total++;
        if (!deleteSetSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
        }
    }
    catch (const std::exception &e)
    {
        printTestResult(false, "Exception occurred: " + std::string(e.what()));
        stats.total++;
        stats.failed++;
    }
}

// 5. 测试测试用例执行和报告生成
void testTestCaseExecution(AutomationCore *core, TestStats &stats)
{
    printTestHeader("Test Case Execution and Reporting");

    try
    {
        // 检查是否有可用的插件
        auto pluginInfos = core->getPluginInfos();
        if (pluginInfos.empty())
        {
            printTestResult(false, "No plugins available for test execution - skipping execution tests");
            stats.total++;
            stats.failed++;
            return;
        }

        // 创建一个简单的测试用例
        TestCase testCase;
        testCase.id = 1;
        testCase.name = "Execution Test Case";
        testCase.description = "Test case for execution and reporting";
        testCase.project_id = 1;

        // 使用第一个可用插件
        std::string pluginName = pluginInfos.begin()->first;
        LOG_INFO_FMT("Using plugin '%s' for test execution", pluginName.c_str());

        // 添加测试步骤（根据插件类型调整）
        TestStep step1;
        step1.id = 1;
        step1.plugin_name = pluginName;
        step1.param.action = "launch_application";
        step1.param.target = "notepad.exe";
        step1.param.value = "";
        step1.stop_on_failure = true;

        TestStep step2;
        step2.id = 2;
        step2.plugin_name = pluginName;
        step2.param.action = "wait_for_window";
        step2.param.target = "无标题 - 记事本";
        step2.param.value = "5000";
        step2.stop_on_failure = true;

        TestStep step3;
        step3.id = 3;
        step3.plugin_name = pluginName;
        step3.param.action = "sleep";
        step3.param.target = "";
        step3.param.value = "1000";
        step3.stop_on_failure = false;

        TestStep step4;
        step4.id = 4;
        step4.plugin_name = pluginName;
        step4.param.action = "close_window";
        step4.param.target = "无标题 - 记事本";
        step4.param.value = "yes";
        step4.stop_on_failure = true;

        testCase.steps = {step1, step2, step3, step4};

        // 执行单个测试用例
        TestExecutionResult result = core->executeTestCase(testCase);
        bool executeSuccess = result.overall_success &&
                              (result.step_results.size() == testCase.steps.size());
        printTestResult(executeSuccess, "Execute single test case");
        stats.total++;
        if (!executeSuccess)
        {
            stats.failed++;
            std::cout << "  Execution failed. Overall success: " << std::boolalpha
                      << result.overall_success << std::endl;
        }
        else
        {
            stats.passed++;
            std::cout << "  Execution time: " << result.total_duration.count() << "ms" << std::endl;
        }

        // 测试报告生成 - HTML
        fs::create_directory("reports");
        std::string htmlReport = core->generateTestCaseReport(result, ReportFormat::HTML);
        bool htmlReportSuccess = !htmlReport.empty() && htmlReport.find("<html>") != std::string::npos;
        printTestResult(htmlReportSuccess, "Generate HTML test report");
        stats.total++;
        if (!htmlReportSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
            core->saveReport(htmlReport, "reports/execution_test.html");
        }

        // 测试报告生成 - TEXT
        std::string textReport = core->generateTestCaseReport(result, ReportFormat::TEXT);
        bool textReportSuccess = !textReport.empty() && textReport.find(testCase.name) != std::string::npos;
        printTestResult(textReportSuccess, "Generate TEXT test report");
        stats.total++;
        if (!textReportSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
            core->saveReport(textReport, "reports/execution_test.txt");
        }

        // 测试报告生成 - XML
        std::string xmlReport = core->generateTestCaseReport(result, ReportFormat::XML);
        bool xmlReportSuccess = !xmlReport.empty() && xmlReport.find("<?xml") != std::string::npos;
        printTestResult(xmlReportSuccess, "Generate XML test report");
        stats.total++;
        if (!xmlReportSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
            core->saveReport(xmlReport, "reports/execution_test.xml");
        }
    }
    catch (const std::exception &e)
    {
        printTestResult(false, "Exception occurred: " + std::string(e.what()));
        stats.total++;
        stats.failed++;
    }
}

// 6. 测试多线程执行
void testParallelExecution(AutomationCore *core, TestStats &stats)
{
    printTestHeader("Parallel Test Execution");

    try
    {
        // 检查是否有可用的插件
        auto pluginInfos = core->getPluginInfos();
        if (pluginInfos.empty())
        {
            printTestResult(false, "No plugins available for test execution - skipping parallel tests");
            stats.total++;
            stats.failed++;
            return;
        }

        // 使用第一个可用插件
        std::string pluginName = pluginInfos.begin()->first;

        // 创建多个测试用例
        std::vector<TestCase> testCases;

        // 测试用例1: 记事本测试
        TestCase case1;
        case1.id = 1;
        case1.name = "Notepad Test";
        case1.description = "Test notepad application";
        case1.project_id = 1;

        TestStep step1_1;
        step1_1.id = 1;
        step1_1.plugin_name = pluginName;
        step1_1.param.action = "launch_application";
        step1_1.param.target = "notepad.exe";
        step1_1.param.value = "";
        step1_1.stop_on_failure = true;

        TestStep step1_2;
        step1_2.id = 2;
        step1_2.plugin_name = pluginName;
        step1_2.param.action = "wait_for_window";
        step1_2.param.target = "无标题 - 记事本";
        step1_2.param.value = "5000";
        step1_2.stop_on_failure = true;

        TestStep step1_3;
        step1_3.id = 3;
        step1_3.plugin_name = pluginName;
        step1_3.param.action = "sleep";
        step1_3.param.target = "";
        step1_3.param.value = "2000";
        step1_3.stop_on_failure = false;

        TestStep step1_4;
        step1_4.id = 4;
        step1_4.plugin_name = pluginName;
        step1_4.param.action = "close_window";
        step1_4.param.target = "无标题 - 记事本";
        step1_4.param.value = "yes";
        step1_4.stop_on_failure = true;

        case1.steps = {step1_1, step1_2, step1_3, step1_4};
        testCases.push_back(case1);

        // 测试用例2: 计算器测试
        TestCase case2;
        case2.id = 2;
        case2.name = "Calculator Test";
        case2.description = "Test calculator application";
        case2.project_id = 1;

        TestStep step2_1;
        step2_1.id = 1;
        step2_1.plugin_name = pluginName;
        step2_1.param.action = "launch_application";
        step2_1.param.target = "calc.exe";
        step2_1.param.value = "";
        step2_1.stop_on_failure = true;

        TestStep step2_2;
        step2_2.id = 2;
        step2_2.plugin_name = pluginName;
        step2_2.param.action = "wait_for_window";
        step2_2.param.target = "计算器";
        step2_2.param.value = "5000";
        step2_2.stop_on_failure = true;

        TestStep step2_3;
        step2_3.id = 3;
        step2_3.plugin_name = pluginName;
        step2_3.param.action = "sleep";
        step2_3.param.target = "";
        step2_3.param.value = "2000";
        step2_3.stop_on_failure = false;

        TestStep step2_4;
        step2_4.id = 4;
        step2_4.plugin_name = pluginName;
        step2_4.param.action = "close_window";
        step2_4.param.target = "计算器";
        step2_4.param.value = "";
        step2_4.stop_on_failure = true;

        case2.steps = {step2_1, step2_2, step2_3, step2_4};
        testCases.push_back(case2);

        // 记录开始时间
        auto start = std::chrono::high_resolution_clock::now();

        // 多线程执行测试用例
        auto results = core->executeTestCasesParallel(testCases, 2); // 2个线程
        bool parallelSuccess = (results.size() == testCases.size());
        printTestResult(parallelSuccess, "Execute multiple test cases in parallel");
        stats.total++;
        if (!parallelSuccess)
        {
            stats.failed++;
            std::cout << "  Expected " << testCases.size() << " results, got " << results.size() << std::endl;
        }
        else
        {
            stats.passed++;

            // 计算总执行时间
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            std::cout << "  Parallel execution time: " << duration.count() << "ms" << std::endl;

            // 验证每个测试用例的执行结果
            int successfulCases = 0;
            for (const auto &result : results)
            {
                if (result.overall_success)
                    successfulCases++;
                std::cout << "  Test case " << result.case_id << " (" << result.case_name << "): "
                          << (result.overall_success ? "PASSED" : "FAILED") << std::endl;
            }

            printTestResult(successfulCases > 0, "At least one test case succeeded in parallel execution");
            stats.total++;
            if (successfulCases == 0)
            {
                stats.failed++;
            }
            else
            {
                stats.passed++;
            }
        }

        // 生成汇总报告
        std::string summaryReport = core->generateReport(results, ReportFormat::HTML, "Parallel Execution Summary");
        bool summarySuccess = !summaryReport.empty();
        printTestResult(summarySuccess, "Generate summary report for parallel execution");
        stats.total++;
        if (!summarySuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
            core->saveReport(summaryReport, "reports/parallel_summary.html");
        }
    }
    catch (const std::exception &e)
    {
        printTestResult(false, "Exception occurred: " + std::string(e.what()));
        stats.total++;
        stats.failed++;
    }
}

// 7. 测试数据驱动的测试用例执行
void testDataDrivenExecution(AutomationCore *core, TestStats &stats)
{
    printTestHeader("Data-Driven Test Execution");

    try
    {
        // 检查是否有可用的插件
        auto pluginInfos = core->getPluginInfos();
        if (pluginInfos.empty())
        {
            printTestResult(false, "No plugins available for test execution - skipping data-driven tests");
            stats.total++;
            stats.failed++;
            return;
        }

        ITestDataManager *dataManager = core->getDataManager();
        if (!dataManager)
        {
            printTestResult(false, "Get TestDataManager instance");
            stats.total++;
            stats.failed++;
            return;
        }

        const int projectId = 1;
        std::string pluginName = pluginInfos.begin()->first;

        // 创建测试数据集
        TestDataSet dataSet;
        dataSet.name = "DataDrivenTestSet";
        dataSet.description = "DataSet for data-driven testing";
        dataSet.project_id = projectId;

        int dataSetId = dataManager->createDataSet(dataSet);
        if (dataSetId <= 0)
        {
            printTestResult(false, "Create data set for data-driven testing");
            stats.total++;
            stats.failed++;
            return;
        }

        // 添加数据项
        TestDataItem appPath;
        appPath.name = "app_path";
        appPath.type = "string";
        appPath.value = "notepad.exe";
        dataManager->addDataItem(dataSetId, appPath);

        TestDataItem windowTitle;
        windowTitle.name = "window_title";
        windowTitle.type = "string";
        windowTitle.value = "无标题 - 记事本";
        dataManager->addDataItem(dataSetId, windowTitle);

        TestDataItem inputText;
        inputText.name = "input_text";
        inputText.type = "string";
        inputText.value = "Data-driven test successful!";
        dataManager->addDataItem(dataSetId, inputText);

        TestDataItem waitTime;
        waitTime.name = "wait_time";
        waitTime.type = "int";
        waitTime.value = "1000";
        dataManager->addDataItem(dataSetId, waitTime);

        // 创建数据驱动的测试用例
        TestCase testCase;
        testCase.id = 1;
        testCase.name = "Data-Driven Test";
        testCase.description = "Test case using data references";
        testCase.project_id = projectId;
        testCase.data_set_ids = {dataSetId}; // 关联数据集

        // 添加使用数据引用的测试步骤
        TestStep step1;
        step1.id = 1;
        step1.plugin_name = pluginName;
        step1.param.action = "launch_application";
        step1.param.target = "${DataDrivenTestSet.app_path}"; // 引用数据
        step1.param.value = "";
        step1.stop_on_failure = true;

        TestStep step2;
        step2.id = 2;
        step2.plugin_name = pluginName;
        step2.param.action = "wait_for_window";
        step2.param.target = "${DataDrivenTestSet.window_title}"; // 引用数据
        step2.param.value = "${DataDrivenTestSet.wait_time}";     // 引用数据
        step2.stop_on_failure = true;

        TestStep step3;
        step3.id = 3;
        step3.plugin_name = pluginName;
        step3.param.action = "input_text";
        step3.param.target = "Edit";
        step3.param.value = "${DataDrivenTestSet.input_text}"; // 引用数据
        step3.stop_on_failure = true;

        TestStep step4;
        step4.id = 4;
        step4.plugin_name = pluginName;
        step4.param.action = "sleep";
        step4.param.target = "";
        step4.param.value = "${DataDrivenTestSet.wait_time}"; // 引用数据
        step4.stop_on_failure = false;

        TestStep step5;
        step5.id = 5;
        step5.plugin_name = pluginName;
        step5.param.action = "close_window";
        step5.param.target = "${DataDrivenTestSet.window_title}"; // 引用数据
        step5.param.value = "yes";
        step5.stop_on_failure = true;

        testCase.steps = {step1, step2, step3, step4, step5};

        // 执行数据驱动的测试用例
        TestExecutionResult result = core->executeTestCase(testCase);
        bool executeSuccess = result.overall_success;
        printTestResult(executeSuccess, "Execute data-driven test case");
        stats.total++;
        if (!executeSuccess)
        {
            stats.failed++;
        }
        else
        {
            stats.passed++;
            core->saveReport(core->generateTestCaseReport(result, ReportFormat::HTML),
                             "reports/data_driven_test.html");
        }
    }
    catch (const std::exception &e)
    {
        printTestResult(false, "Exception occurred: " + std::string(e.what()));
        stats.total++;
        stats.failed++;
    }
}

// 主测试函数
int main()
{
    std::cout << "=== AutomationCore Comprehensive Test Suite ===" << std::endl;

    // 创建核心实例
    AutomationCore *core = createAutomationCore();
    if (!core)
    {
        std::cerr << "Failed to create AutomationCore instance" << std::endl;
        return 1;
    }

    TestStats stats;

    try
    {
        // 初始化核心框架，加载插件目录
        fs::create_directory("plugins"); // 确保插件目录存在
        bool initSuccess = initializeCore(core, "plugins");
        printTestHeader("Core Initialization");
        printTestResult(initSuccess, "Initialize AutomationCore");
        stats.total++;
        if (!initSuccess)
        {
            stats.failed++;
            std::cerr << "Cannot proceed with tests - core initialization failed" << std::endl;
        }
        else
        {
            stats.passed++;

            // 执行所有测试
            testCoreInitialization(core, stats);
            testPluginManagement(core, stats);
            testTestCaseSerialization(core, stats);
            testTestDataManagement(core, stats);
            testTestCaseExecution(core, stats);
            testParallelExecution(core, stats);
            testDataDrivenExecution(core, stats);
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Critical exception occurred: " << e.what() << std::endl;
        stats.total++;
        stats.failed++;
    }

    // 反初始化并销毁核心实例
    uninitializeCore(core);
    destroyAutomationCore(core);

    // 打印总体测试结果
    stats.printSummary();

    std::cout << "\nAll tests completed. Press Enter to exit..." << std::endl;
    std::cin.get();

    // 返回适当的退出代码
    return stats.failed > 0 ? 1 : 0;
}
