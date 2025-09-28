#pragma once

#include <string>
#include <vector>
#include "TestEngine.h"


enum class ReportFormat
{
    TEXT, // 文本格式
    HTML, // HTML格式
    XML   // XML格式
};


/**
 * @brief 报告生成器类
 * 负责生成各种格式的测试报告
 */
class ReportGenerator
{
public:
    /**
     * @brief 构造函数
     */
    ReportGenerator() = default;

    /**
     * @brief 析构函数
     */
    ~ReportGenerator() = default;

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
    std::string generateSummaryReport(const std::vector<TestExecutionResult> &results,
                                      ReportFormat format,
                                      const std::string &title = "Test Automation Report");

private:
    /**
     * @brief 生成单个测试用例的文本报告
     * @param result 测试执行结果
     * @return 文本报告内容
     */
    std::string generateTestCaseTextReport(const TestExecutionResult &result);

    /**
     * @brief 生成单个测试用例的HTML报告
     * @param result 测试执行结果
     * @return HTML报告内容
     */
    std::string generateTestCaseHtmlReport(const TestExecutionResult &result);

    /**
     * @brief 生成单个测试用例的XML报告
     * @param result 测试执行结果
     * @return XML报告内容
     */
    std::string generateTestCaseXmlReport(const TestExecutionResult &result);

    /**
     * @brief 生成汇总文本报告
     * @param results 测试执行结果列表
     * @param title 报告标题
     * @return 文本报告内容
     */
    std::string generateSummaryTextReport(const std::vector<TestExecutionResult> &results,
                                          const std::string &title);

    /**
     * @brief 生成汇总HTML报告
     * @param results 测试执行结果列表
     * @param title 报告标题
     * @return HTML报告内容
     */
    std::string generateSummaryHtmlReport(const std::vector<TestExecutionResult> &results,
                                          const std::string &title);

    /**
     * @brief 生成汇总XML报告
     * @param results 测试执行结果列表
     * @param title 报告标题
     * @return XML报告内容
     */
    std::string generateSummaryXmlReport(const std::vector<TestExecutionResult> &results,
                                         const std::string &title);

    /**
     * @brief 计算测试结果统计信息
     * @param results 测试执行结果列表
     * @param total 总用例数
     * @param passed 通过用例数
     * @param failed 失败用例数
     * @param totalDuration 总执行时间
     */
    void calculateStatistics(const std::vector<TestExecutionResult> &results,
                             int &total, int &passed, int &failed,
                             std::chrono::milliseconds &totalDuration);

    /**
     * @brief 获取当前时间字符串
     * @return 格式化的时间字符串
     */
    std::string getCurrentDateTime();

    std::string m_indentation = "    ";

};
