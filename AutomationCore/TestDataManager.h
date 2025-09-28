#pragma once


#include "ITestDataManager.h"

// 测试数据管理器实现类
class TestDataManager : public ITestDataManager {
private:
    std::map<int, TestDataSet> dataSets_;          // 所有数据集，ID为键
    std::map<std::string, int> dataSetNameMap_;    // 数据集名称到ID的映射
    int nextDataSetId_ = 1;                        // 下一个数据集ID
    int nextDataItemId_ = 1;                       // 下一个数据项ID

    // 生成唯一ID
    int generateDataSetId();
    int generateDataItemId();

    // 检查数据集是否存在
    void checkDataSetExists(int dataSetId);
    std::string getCurrentTime();

public:
    TestDataManager() = default;
    ~TestDataManager() override = default;

    // 数据集管理
    int createDataSet(const TestDataSet& dataSet) override;
    bool updateDataSet(const TestDataSet& dataSet) override;
    bool deleteDataSet(int dataSetId) override;
    std::shared_ptr<TestDataSet> getDataSet(int dataSetId) override;
    std::vector<std::shared_ptr<TestDataSet>> getProjectDataSets(int projectId) override;
    std::vector<std::shared_ptr<TestDataSet>> getAllDataSets() override;

    // 数据项管理
    int addDataItem(int dataSetId, const TestDataItem& dataItem) override;
    bool updateDataItem(int dataSetId, const TestDataItem& dataItem) override;
    bool removeDataItem(int dataSetId, int dataItemId) override;
    bool removeDataItembyName(int dataSetId, const std::string& name) override;
    std::shared_ptr<TestDataItem> getDataItem(int dataSetId, int dataItemId) override;
    std::shared_ptr<TestDataItem> getDataItemByName(int dataSetId, const std::string& name) override;

    // 数据引用解析
    std::string resolveDataReference(const std::string& reference) override;
    std::string substituteDataReferences(const std::string& input) override;

    // 导入导出
    bool importDataSetFromFile(const std::string& filePath, int projectId) override;
    bool exportDataSetToFile(int dataSetId, const std::string& filePath) override;
};
