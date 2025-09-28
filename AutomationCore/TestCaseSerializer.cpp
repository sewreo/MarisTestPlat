#include "TestCaseSerializer.h"
#include <fstream>
#include <stdexcept>
#include <sstream>

// 测试用例序列化
json TestCaseSerializer::serializeTestCase(const TestCase& testCase) {
    json j;
    j["id"] = testCase.id;
    j["name"] = testCase.name;
    j["description"] = testCase.description;
    j["project_id"] = testCase.project_id;
    j["steps"] = json::array();

    // 序列化测试步骤
    for (const auto& step : testCase.steps) {
        json stepJson;
        stepJson["id"] = step.id;
        stepJson["plugin_name"] = step.plugin_name;
        stepJson["action"] = step.param.action;
        stepJson["target"] = step.param.target;
        stepJson["value"] = step.param.value;
        stepJson["stop_on_failure"] = step.stop_on_failure;
        j["steps"].push_back(stepJson);
    }
    /*
    if (!testCase.created_at.empty()) {
        j["created_at"] = testCase.created_at;
    }

    if (!testCase.last_modified.empty()) {
        j["last_modified"] = testCase.last_modified;
    }
    */
    return j;
}

// 测试用例列表序列化
json TestCaseSerializer::serializeTestCases(const std::vector<TestCase>& testCases) {
    json j = json::array();

    for (const auto& testCase : testCases) {
        j.push_back(serializeTestCase(testCase));
    }

    return j;
}

// 序列化单个测试用例到文件
bool TestCaseSerializer::serializeToFile(const TestCase& testCase, const std::string& filePath) {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        json j = serializeTestCase(testCase);
        file << j.dump(4); // 带缩进的格式化输出
        file.close();
        return true;
    }
    catch (...) {
        return false;
    }
}

// 序列化测试用例列表到文件
bool TestCaseSerializer::serializeToFile(const std::vector<TestCase>& testCases, const std::string& filePath) {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return false;
        }

        json j = serializeTestCases(testCases);
        file << j.dump(4);
        file.close();
        return true;
    }
    catch (...) {
        return false;
    }
}

// 从JSON反序列化测试用例
TestCase TestCaseSerializer::deserializeTestCase(const json& j) {
    TestCase testCase;

    if (j.contains("id")) {
        testCase.id = j["id"];
    }

    if (j.contains("name")) {
        testCase.name = j["name"];
    }

    if (j.contains("description")) {
        testCase.description = j["description"];
    }

    if (j.contains("project_id")) {
        testCase.project_id = j["project_id"];
    }

    if (j.contains("steps") && j["steps"].is_array()) {
        for (const auto& stepJson : j["steps"]) {
            TestStep step;

            if (stepJson.contains("id")) {
                step.id = stepJson["id"];
            }

            if (stepJson.contains("plugin_name")) {
                step.plugin_name = stepJson["plugin_name"];
            }

            if (stepJson.contains("action")) {
                step.param.action = stepJson["action"];
            }

            if (stepJson.contains("target")) {
                step.param.target = stepJson["target"];
            }

            if (stepJson.contains("value")) {
                step.param.value = stepJson["value"];
            }

            if (stepJson.contains("stop_on_failure")) {
                step.stop_on_failure = stepJson["stop_on_failure"];
            }

            testCase.steps.push_back(step);
        }
    }
/*
    if (j.contains("created_at")) {
        testCase.created_at = j["created_at"];
    }

    if (j.contains("last_modified")) {
        testCase.last_modified = j["last_modified"];
    }
*/
    return testCase;
}

// 从JSON数组反序列化测试用例列表
std::vector<TestCase> TestCaseSerializer::deserializeTestCases(const json& j) {
    std::vector<TestCase> testCases;

    if (j.is_array()) {
        for (const auto& testCaseJson : j) {
            testCases.push_back(deserializeTestCase(testCaseJson));
        }
    }

    return testCases;
}

// 从文件反序列化单个测试用例
TestCase TestCaseSerializer::deserializeSingleCaseFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    try {
        json j;
        file >> j;
        file.close();
        return deserializeTestCase(j);
    }
    catch (const std::exception& e) {
        file.close();
        throw std::runtime_error("Failed to deserialize test case: " + std::string(e.what()));
    }
}

// 从文件反序列化测试用例列表
std::vector<TestCase> TestCaseSerializer::deserializeCasesFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath);
    }

    try {
        json j;
        file >> j;
        file.close();
        return deserializeTestCases(j);
    }
    catch (const std::exception& e) {
        file.close();
        throw std::runtime_error("Failed to deserialize test cases: " + std::string(e.what()));
    }
}