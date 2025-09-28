#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

// 测试数据项结构
struct TestDataItem {
    int id;                     // 数据项ID
    std::string name;           // 数据项名称（用于引用）
    std::string type;           // 数据类型（string, int, float, bool, json等）
    std::string value;          // 数据值（以字符串形式存储，使用时转换）
    std::string description;    // 数据项描述
    int project_id;             // 所属项目ID
    std::string created_at;     // 创建时间
    std::string last_modified;  // 最后修改时间
};
// 测试数据集结构
struct TestDataSet {
    int id;                         // 数据集ID
    std::string name;               // 数据集名称
    std::string description;        // 数据集描述
    int project_id;                 // 所属项目ID
    std::vector<TestDataItem> items;// 数据集中的所有数据项
    std::string created_at;         // 创建时间
    std::string last_modified;      // 最后修改时间
};


// 测试数据管理异常
class TestDataException : public std::runtime_error {
public:
    explicit TestDataException(const std::string& message) : std::runtime_error(message) {}
};

// 测试数据管理器接口
class ITestDataManager {
public:
    virtual ~ITestDataManager() = default;

    // 数据集管理
    virtual int createDataSet(const TestDataSet& dataSet) = 0;
    virtual bool updateDataSet(const TestDataSet& dataSet) = 0;
    virtual bool deleteDataSet(int dataSetId) = 0;
    virtual std::shared_ptr<TestDataSet> getDataSet(int dataSetId) = 0;
    virtual std::vector<std::shared_ptr<TestDataSet>> getProjectDataSets(int projectId) = 0;
    virtual std::vector<std::shared_ptr<TestDataSet>> getAllDataSets() = 0;

    // 数据项管理
    virtual int addDataItem(int dataSetId, const TestDataItem& dataItem) = 0;
    virtual bool updateDataItem(int dataSetId, const TestDataItem& dataItem) = 0;
    virtual bool removeDataItem(int dataSetId, int dataItemId) = 0;
    virtual bool removeDataItembyName(int dataSetId, const std::string& name) = 0;

    virtual std::shared_ptr<TestDataItem> getDataItem(int dataSetId, int dataItemId) = 0;
    virtual std::shared_ptr<TestDataItem> getDataItemByName(int dataSetId, const std::string& name) = 0;

    // 数据引用解析
    virtual std::string resolveDataReference(const std::string& reference) = 0;
    virtual std::string substituteDataReferences(const std::string& input) = 0;

    // 导入导出
    virtual bool importDataSetFromFile(const std::string& filePath, int projectId) = 0;
    virtual bool exportDataSetToFile(int dataSetId, const std::string& filePath) = 0;
};