#pragma once

#include <string>
#include <vector>
#include "TestEngine.h"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

/**
 * @brief 测试用例序列化器
 * 负责测试用例的序列化和反序列化
 */
class TestCaseSerializer
{
public:
    /**
     * @brief 构造函数
     */
    TestCaseSerializer() = default;

    /**
     * @brief 析构函数
     */
    ~TestCaseSerializer() = default;

    /**
     * @brief 将测试用例序列化为JSON
     * @param testCase 测试用例
     * @return JSON对象
     */
    json serializeTestCase(const TestCase &testCase);

    /**
     * @brief 将测试用例列表序列化为JSON
     * @param testCases 测试用例列表
     * @return JSON数组
     */
    json serializeTestCases(const std::vector<TestCase> &testCases);

    /**
     * @brief 将测试用例序列化为JSON并保存到文件
     * @param testCase 测试用例
     * @param filePath 文件路径
     * @return 成功返回true，否则返回false
     */
    bool serializeToFile(const TestCase &testCase, const std::string &filePath);

    /**
     * @brief 将测试用例列表序列化为JSON并保存到文件
     * @param testCases 测试用例列表
     * @param filePath 文件路径
     * @return 成功返回true，否则返回false
     */
    bool serializeToFile(const std::vector<TestCase> &testCases, const std::string &filePath);

    /**
     * @brief 从JSON反序列化测试用例
     * @param j JSON对象
     * @return 测试用例
     */
    TestCase deserializeTestCase(const json &j);

    /**
     * @brief 从JSON数组反序列化测试用例列表
     * @param j JSON数组
     * @return 测试用例列表
     */
    std::vector<TestCase> deserializeTestCases(const json &j);

    /**
     * @brief 从JSON文件反序列化测试用例
     * @param filePath 文件路径
     * @return 测试用例
     */
    TestCase deserializeSingleCaseFromFile(const std::string &filePath);

    /**
     * @brief 从JSON文件反序列化测试用例列表
     * @param filePath 文件路径
     * @return 测试用例列表
     */
    std::vector<TestCase> deserializeCasesFromFile(const std::string &filePath);
};
