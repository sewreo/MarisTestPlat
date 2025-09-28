#include "TestEngine.h"
#include <iostream>
#include <sstream>
#include <iomanip>

// 时间工具
std::string TestEngine::getCurrentTime()
{
    auto now = std::time(nullptr);
    std::tm localTime{}; // 栈上创建tm对象
#ifdef _WIN32
    localtime_s(&localTime, &now); // Windows安全函数
#else
    localtime_r(&localTime, &now); // Linux/macOS线程安全函数
#endif
    std::stringstream ss;
    ss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

TestEngine::TestEngine(IPluginManager* pluginManager, ITestDataManager* testDataManager)
    : plugin_manager_(pluginManager), testDataManager_(testDataManager)
{
    // 添加空指针检查
    if (!pluginManager) {
        throw std::invalid_argument("Plugin manager cannot be null");
    }
    if (!testDataManager) {
        throw std::invalid_argument("Test data manager cannot be null");
    }
}

TestExecutionResult TestEngine::executeTestCase(const TestCase &testCase)
{
    TestExecutionResult result;
    result.case_id = testCase.id;
    result.case_name = testCase.name;
    result.start_time = std::chrono::system_clock::now();

    log("Starting execution of test case: " + testCase.name);

    try
    {
        // 执行前置操作
        if (!testCase.setup_script.empty())
        {
            log("Executing setup script");
            if (!executeSetup(testCase.setup_script))
            {
                result.overall_success = false;
                result.error_message = "Setup script failed";
                log("Setup script failed");
                result.end_time = std::chrono::system_clock::now();
                result.total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    result.end_time - result.start_time);
                return result;
            }
        }

        // 执行测试步骤
        for (const auto &step : testCase.steps)
        {
            StepExecutionResult stepResult = executeTestStep(step);
            result.step_results.push_back(stepResult);

            // 如果步骤失败且设置了失败时停止，则终止执行
            if (!stepResult.result.success && step.stop_on_failure)
            {
                log("Step " + std::to_string(step.id) + " failed. Stopping test case execution.");
                break;
            }
        }

        // 执行后置操作
        if (!testCase.teardown_script.empty())
        {
            log("Executing teardown script");
            executeTeardown(testCase.teardown_script);
        }

        // 判断整体执行结果
        result.overall_success = true;
        for (const auto &stepResult : result.step_results)
        {
            if (!stepResult.result.success &&
                !testCase.steps[&stepResult - &result.step_results[0]].is_optional)
            {
                result.overall_success = false;
                break;
            }
        }

        log("Test case " + testCase.name + " execution completed. " +
            (result.overall_success ? "Success" : "Failed"));
    }
    catch (const std::exception &e)
    {
        result.overall_success = false;
        result.error_message = "Exception occurred: " + std::string(e.what());
        log("Exception in test case execution: " + std::string(e.what()));
    }
    catch (...)
    {
        result.overall_success = false;
        result.error_message = "Unknown exception occurred";
        log("Unknown exception in test case execution");
    }

    // 计算总执行时间
    result.end_time = std::chrono::system_clock::now();
    result.total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        result.end_time - result.start_time);

    return result;
}

std::vector<TestExecutionResult> TestEngine::executeTestCases(const std::vector<TestCase> &testCases)
{
    std::vector<TestExecutionResult> results;
    results.reserve(testCases.size());

    for (const auto &testCase : testCases)
    {
        results.push_back(executeTestCase(testCase));
    }

    return results;
}

StepExecutionResult TestEngine::executeTestStep(const TestStep &step)
{
    StepExecutionResult result;
    result.step_id = step.id;
    result.start_time = std::chrono::system_clock::now();

    std::stringstream ss;
    ss << "Executing step " << step.id << ": "
       << step.param.action << " on " << step.param.target;
    log(ss.str());

    try
    {
        // 获取插件
        IAutomationPlugin *plugin = plugin_manager_->getPlugin(step.plugin_name);
        if (!plugin)
        {
            result.result.success = false;
            result.result.error_code = -1;
            result.result.message = "Plugin not found: " + step.plugin_name;
            log("Error: " + result.result.message);
        }
        else
        {
            // 检查插件是否支持该动作
            const auto &supportedActions = plugin->supportedActions();
            if (std::find(supportedActions.begin(), supportedActions.end(), step.param.action) == supportedActions.end())
            {
                result.result.success = false;
                result.result.error_code = -2;
                result.result.message = "Plugin " + step.plugin_name + " does not support action: " + step.param.action;
                log("Error: " + result.result.message);
            }
            else
            {
                // 执行步骤
                result.result = plugin->executeStep(step.param);

                if (result.result.success)
                {
                    std::stringstream ss;
                    ss << "Step " << step.id << " completed successfully";
                    log(ss.str());
                    if (verbose_logging_ && !result.result.extra_data.empty())
                    {
                        std::stringstream ss_data;
                        ss_data << "Step " << step.id << " returned data: " << result.result.extra_data;
                        log(ss_data.str());
                    }
                }
                else
                {
                    std::stringstream ss;
                    ss << "Step " << step.id << " failed: " << result.result.message;
                    log(ss.str());
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        result.result.success = false;
        result.result.error_code = -3;
        result.result.message = "Exception occurred: " + std::string(e.what());
        log("Exception in step execution: " + std::string(e.what()));
    }
    catch (...)
    {
        result.result.success = false;
        result.result.error_code = -4;
        result.result.message = "Unknown exception occurred";
        log("Unknown exception in step execution");
    }

    // 计算步骤执行时间
    auto end_time = std::chrono::system_clock::now();
    result.duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - result.start_time);

    if (verbose_logging_)
    {
        std::stringstream ss_data;
        ss_data << "Step " << step.id << " execution time: " << result.duration.count() << "ms";
        log(ss_data.str());
    }

    return result;
}

void TestEngine::log(const std::string &message)
{
    // 输出到控制台
    std::cout << "[" << getCurrentTime() << "] "
              << message << std::endl;

    // 记录到日志字符串
    execution_log_ += message + "\n";
}

bool TestEngine::executeSetup(const std::string &setup_script)
{
    // 此处实现前置脚本执行逻辑
    // 实际项目中可能需要集成脚本引擎(如Lua, Python等)
    log("Setup script execution: " + setup_script);

    // 示例：简单返回成功
    return true;
}

void TestEngine::executeTeardown(const std::string &teardown_script)
{
    // 此处实现后置脚本执行逻辑
    log("Teardown script execution: " + teardown_script);
}
