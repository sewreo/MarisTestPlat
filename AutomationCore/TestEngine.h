#pragma once
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "IAutomationPlugin.h"
#include "PluginManager.h"
#include "TestDataManager.h"

// 前向声明
struct TestCase;
struct TestResult;
struct TestStep;

/**
 * @brief 测试步骤结构
 */
struct TestStep {
    int id = 0;                      // 步骤ID
    std::string plugin_name;         // 插件名称
    StepParam param;                 // 步骤参数
    bool is_optional = false;        // 是否为可选步骤
    bool stop_on_failure = true;     // 失败时是否停止
};

/**
 * @brief 测试用例结构
 */
struct TestCase {
    int id = 0;                      // 用例ID
    std::string name;                // 用例名称
    std::string description;         // 用例描述
    std::vector<TestStep> steps;     // 测试步骤列表
    int project_id = 0;              // 所属项目ID
    std::string setup_script;        // 前置脚本
    std::string teardown_script;     // 后置脚本
    std::string  created_at;
    std::string last_modified;
    std::vector<int> data_set_ids ; // 关联数据集

};

/**
 * @brief 测试步骤执行结果
 */
struct StepExecutionResult {
    int step_id = 0;                 // 步骤ID
    StepResult result;               // 执行结果
    std::chrono::milliseconds duration; // 执行时长
    std::chrono::system_clock::time_point start_time; // 开始时间
};

/**
 * @brief 测试用例执行结果
 */
struct TestExecutionResult {
    int case_id = 0;                 // 用例ID
    std::string case_name;           // 用例名称
    bool overall_success = false;    // 整体是否成功
    std::vector<StepExecutionResult> step_results; // 步骤结果列表
    std::chrono::milliseconds total_duration; // 总执行时长
    std::chrono::system_clock::time_point start_time; // 开始时间
    std::chrono::system_clock::time_point end_time;   // 结束时间
    std::string error_message;       // 错误信息（如果有）
};

/**
 * @brief 测试引擎类，负责执行测试用例
 */
class TestEngine {
public:
    /**
     * @brief 构造函数
     * @param pluginManager 插件管理器引用
     */
    explicit TestEngine(IPluginManager* pluginManager, ITestDataManager* testDataManager);

    /**
     * @brief 执行单个测试用例
     * @param testCase 测试用例
     * @return 执行结果
     */
    TestExecutionResult executeTestCase(const TestCase& testCase);

    /**
     * @brief 执行多个测试用例
     * @param testCases 测试用例列表
     * @return 执行结果列表
     */
    std::vector<TestExecutionResult> executeTestCases(const std::vector<TestCase>& testCases);

    /**
     * @brief 执行单个测试步骤
     * @param step 测试步骤
     * @return 执行结果
     */
    StepExecutionResult executeTestStep(const TestStep& step);

    /**
     * @brief 设置是否启用详细日志
     * @param enable 是否启用
     */
    void setVerboseLogging(bool enable) { verbose_logging_ = enable; }

    /**
     * @brief 获取测试执行日志
     * @return 日志内容
     */
    std::string getExecutionLog() const { return execution_log_; }

    /**
     * @brief 清空执行日志
     */
    void clearExecutionLog() { execution_log_.clear(); }

private:
    IPluginManager* plugin_manager_;   // 插件管理器引用
    ITestDataManager* testDataManager_;
    bool verbose_logging_ = false;    // 是否启用详细日志
    std::string execution_log_;       // 执行日志

    /**
     * @brief 记录日志
     * @param message 日志消息
     */
    void log(const std::string& message);

    /**
     * @brief 执行前置操作
     * @param setup_script 前置脚本
     * @return 是否成功
     */
    bool executeSetup(const std::string& setup_script);

    /**
     * @brief 执行后置操作
     * @param teardown_script 后置脚本
     */
    void executeTeardown(const std::string& teardown_script);

    std::string getCurrentTime();
};
