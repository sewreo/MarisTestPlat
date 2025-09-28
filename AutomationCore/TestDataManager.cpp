#include "TestDataManager.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <ctime>
#include <iomanip>
#include "nlohmann/json.hpp"

using json = nlohmann::json;


// 时间工具
std::string TestDataManager::getCurrentTime() {
    auto now = std::time(nullptr);
    std::tm localTime{};  // 栈上创建tm对象
#ifdef _WIN32
    localtime_s(&localTime, &now);  // Windows安全函数
#else
    localtime_r(&localTime, &now);  // Linux/macOS线程安全函数
#endif
    std::stringstream ss;
    ss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

// 生成唯一数据集ID
int TestDataManager::generateDataSetId() {
    return nextDataSetId_++;
}

// 生成唯一数据项ID
int TestDataManager::generateDataItemId() {
    return nextDataItemId_++;
}

// 检查数据集是否存在
void TestDataManager::checkDataSetExists(int dataSetId) {
    if (dataSets_.find(dataSetId) == dataSets_.end()) {
        throw TestDataException("DataSet with ID " + std::to_string(dataSetId) + " does not exist");
    }
}

// 创建数据集
int TestDataManager::createDataSet(const TestDataSet& dataSet) {
    // 检查名称是否已存在
    if (dataSetNameMap_.find(dataSet.name) != dataSetNameMap_.end()) {
        throw TestDataException("DataSet with name '" + dataSet.name + "' already exists");
    }

    // 创建新数据集
    TestDataSet newDataSet = dataSet;
    newDataSet.id = generateDataSetId();

    // 设置时间戳
    newDataSet.created_at = getCurrentTime();
    newDataSet.last_modified = newDataSet.created_at;

    // 保存数据集
    dataSets_[newDataSet.id] = newDataSet;
    dataSetNameMap_[newDataSet.name] = newDataSet.id;

    return newDataSet.id;
}

// 更新数据集
bool TestDataManager::updateDataSet(const TestDataSet& dataSet) {
    checkDataSetExists(dataSet.id);

    // 如果名称改变，更新名称映射
    auto existingDataSet = dataSets_[dataSet.id];
    if (existingDataSet.name != dataSet.name) {
        if (dataSetNameMap_.find(dataSet.name) != dataSetNameMap_.end()) {
            throw TestDataException("DataSet with name '" + dataSet.name + "' already exists");
        }

        dataSetNameMap_.erase(existingDataSet.name);
        dataSetNameMap_[dataSet.name] = dataSet.id;
    }

    // 更新时间戳
    TestDataSet updatedDataSet = dataSet;
    updatedDataSet.last_modified = getCurrentTime();

    // 保留创建时间
    updatedDataSet.created_at = existingDataSet.created_at;

    dataSets_[dataSet.id] = updatedDataSet;
    return true;
}

// 删除数据集
bool TestDataManager::deleteDataSet(int dataSetId) {
    checkDataSetExists(dataSetId);

    std::string name = dataSets_[dataSetId].name;
    dataSetNameMap_.erase(name);
    dataSets_.erase(dataSetId);

    return true;
}

// 获取数据集
std::shared_ptr<TestDataSet> TestDataManager::getDataSet(int dataSetId) {
    checkDataSetExists(dataSetId);
    return std::make_shared<TestDataSet>(dataSets_[dataSetId]);
}

// 获取项目的所有数据集
std::vector<std::shared_ptr<TestDataSet>> TestDataManager::getProjectDataSets(int projectId) {
    std::vector<std::shared_ptr<TestDataSet>> result;

    for (const auto& pair : dataSets_) {
        if (pair.second.project_id == projectId) {
            result.push_back(std::make_shared<TestDataSet>(pair.second));
        }
    }

    return result;
}

// 获取所有数据集
std::vector<std::shared_ptr<TestDataSet>> TestDataManager::getAllDataSets() {
    std::vector<std::shared_ptr<TestDataSet>> result;

    for (const auto& pair : dataSets_) {
        result.push_back(std::make_shared<TestDataSet>(pair.second));
    }

    return result;
}

// 添加数据项
int TestDataManager::addDataItem(int dataSetId, const TestDataItem& dataItem) {
    checkDataSetExists(dataSetId);

    TestDataSet& dataSet = dataSets_[dataSetId];

    // 检查数据项名称是否已存在
    for (const auto& item : dataSet.items) {
        if (item.name == dataItem.name) {
            throw TestDataException("DataItem with name '" + dataItem.name + "' already exists in DataSet " + std::to_string(dataSetId));
        }
    }

    // 创建新数据项
    TestDataItem newItem = dataItem;
    newItem.id = generateDataItemId();
    newItem.project_id = dataSet.project_id;

    // 设置时间戳
    newItem.created_at = getCurrentTime();
    newItem.last_modified = newItem.created_at;

    // 添加到数据集
    dataSet.items.push_back(newItem);

    // 更新数据集的最后修改时间
    dataSet.last_modified = newItem.created_at;

    return newItem.id;
}

// 更新数据项
bool TestDataManager::updateDataItem(int dataSetId, const TestDataItem& dataItem) {
    checkDataSetExists(dataSetId);

    TestDataSet& dataSet = dataSets_[dataSetId];
    bool found = false;

    for (auto& item : dataSet.items) {
        if (item.id == dataItem.id) {
            // 检查名称是否已被其他数据项使用
            if (item.name != dataItem.name) {
                for (const auto& otherItem : dataSet.items) {
                    if (otherItem.id != dataItem.id && otherItem.name == dataItem.name) {
                        throw TestDataException("DataItem with name '" + dataItem.name + "' already exists in DataSet " + std::to_string(dataSetId));
                    }
                }
            }

         
      

            // 保留创建时间
            std::string created_at = item.created_at;

            item = dataItem;
            item.created_at = created_at;

            // 更新时间戳
            item.last_modified = getCurrentTime();

            // 更新数据集的最后修改时间
            dataSet.last_modified = item.last_modified;

            found = true;
            break;
        }
    }

    if (!found) {
        throw TestDataException("DataItem with ID " + std::to_string(dataItem.id) + " not found in DataSet " + std::to_string(dataSetId));
    }

    return true;
}

// 移除数据项
bool TestDataManager::removeDataItem(int dataSetId, int dataItemId) {
    checkDataSetExists(dataSetId);

    TestDataSet& dataSet = dataSets_[dataSetId];
    auto it = std::remove_if(dataSet.items.begin(), dataSet.items.end(),
        [dataItemId](const TestDataItem& item) {
            return item.id == dataItemId;
        });

    if (it != dataSet.items.end()) {
        dataSet.items.erase(it, dataSet.items.end());

        // 更新数据集的最后修改时间
        dataSet.last_modified  = getCurrentTime();

        return true;
    }

    return false;
}

//通过名称移除测试项数据
bool TestDataManager::removeDataItembyName(int dataSetId, const std::string& name)  {
    checkDataSetExists(dataSetId);
    auto dataItem = getDataItemByName(dataSetId, name);
    int dataItemId = dataItem->id;
    return removeDataItem(dataSetId, dataItemId);  
}


// 通过ID获取数据项
std::shared_ptr<TestDataItem> TestDataManager::getDataItem(int dataSetId, int dataItemId) {
    checkDataSetExists(dataSetId);

    const TestDataSet& dataSet = dataSets_[dataSetId];

    for (const auto& item : dataSet.items) {
        if (item.id == dataItemId) {
            return std::make_shared<TestDataItem>(item);
        }
    }

    throw TestDataException("DataItem with ID " + std::to_string(dataItemId) + " not found in DataSet " + std::to_string(dataSetId));
}

// 通过名称获取数据项
std::shared_ptr<TestDataItem> TestDataManager::getDataItemByName(int dataSetId, const std::string& name) {
    checkDataSetExists(dataSetId);

    const TestDataSet& dataSet = dataSets_[dataSetId];

    for (const auto& item : dataSet.items) {
        if (item.name == name) {
            return std::make_shared<TestDataItem>(item);
        }
    }

    throw TestDataException("DataItem with name '" + name + "' not found in DataSet " + std::to_string(dataSetId));
}

// 解析数据引用（格式：${dataset_name.item_name}）
std::string TestDataManager::resolveDataReference(const std::string& reference) {
    // 检查引用格式
    std::regex refRegex("\\$\\{([^.]+)\\.([^}]+)\\}");
    std::smatch match;

    if (!std::regex_match(reference, match, refRegex) || match.size() != 3) {
        throw TestDataException("Invalid data reference format: " + reference + ". Use ${dataset_name.item_name}");
    }

    std::string dataSetName = match[1];
    std::string itemName = match[2];

    // 查找数据集
    if (dataSetNameMap_.find(dataSetName) == dataSetNameMap_.end()) {
        throw TestDataException("DataSet with name '" + dataSetName + "' not found");
    }

    int dataSetId = dataSetNameMap_[dataSetName];

    // 查找数据项
    try {
        auto dataItem = getDataItemByName(dataSetId, itemName);
        return dataItem->value;
    }
    catch (const TestDataException& e) {
        throw TestDataException("Failed to resolve reference " + reference + ": " + e.what());
    }
}

// 替换文本中的所有数据引用
std::string TestDataManager::substituteDataReferences(const std::string& input) {
    std::regex refRegex("\\$\\{([^.]+)\\.([^}]+)\\}");
    std::string result = input;
    std::smatch match;

    // 查找所有匹配的引用
    std::string temp = input;
    while (std::regex_search(temp, match, refRegex)) {
        if (match.size() == 3) {
            std::string fullMatch = match[0];
            std::string dataSetName = match[1];
            std::string itemName = match[2];

            try {
                // 解析引用并替换
                if (dataSetNameMap_.find(dataSetName) != dataSetNameMap_.end()) {
                    int dataSetId = dataSetNameMap_[dataSetName];
                    auto dataItem = getDataItemByName(dataSetId, itemName);

                    // 替换所有相同的引用
                    size_t pos = 0;
                    while ((pos = result.find(fullMatch, pos)) != std::string::npos) {
                        result.replace(pos, fullMatch.length(), dataItem->value);
                        pos += dataItem->value.length();
                    }
                }
            }
            catch (const TestDataException&) {
                // 引用解析失败，保留原始引用并继续处理其他引用
            }
        }

        temp = match.suffix().str();
    }

    return result;
}

// 从文件导入数据集
bool TestDataManager::importDataSetFromFile(const std::string& filePath, int projectId) {
    try {
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw TestDataException("Failed to open file: " + filePath);
        }

        json j;
        file >> j;

        // 检查JSON结构
        if (!j.contains("name") || !j["name"].is_string()) {
            throw TestDataException("Invalid DataSet format: missing or invalid 'name' field");
        }

        TestDataSet dataSet;
        dataSet.name = j["name"].get<std::string>();
        dataSet.description = j.contains("description") ? j["description"].get<std::string>() : "";
        dataSet.project_id = projectId;

        // 导入数据项
        if (j.contains("items") && j["items"].is_array()) {
            for (const auto& itemJson : j["items"]) {
                if (itemJson.contains("name") && itemJson["name"].is_string() &&
                    itemJson.contains("type") && itemJson["type"].is_string() &&
                    itemJson.contains("value")) {

                    TestDataItem item;
                    item.name = itemJson["name"].get<std::string>();
                    item.type = itemJson["type"].get<std::string>();
                    item.value = itemJson["value"].dump(); // 保存原始JSON值
                    item.description = itemJson.contains("description") ? itemJson["description"].get<std::string>() : "";

                    dataSet.items.push_back(item);
                }
            }
        }

        // 创建数据集
        createDataSet(dataSet);
        return true;
    }
    catch (const std::exception& e) {
        throw TestDataException("Failed to import DataSet: " + std::string(e.what()));
    }
}

// 导出数据集到文件
bool TestDataManager::exportDataSetToFile(int dataSetId, const std::string& filePath) {
    try {
        checkDataSetExists(dataSetId);
        const TestDataSet& dataSet = dataSets_[dataSetId];

        json j;
        j["name"] = dataSet.name;
        j["description"] = dataSet.description;
        j["project_id"] = dataSet.project_id;
        j["created_at"] = dataSet.created_at;
        j["last_modified"] = dataSet.last_modified;

        // 导出数据项
        json itemsJson = json::array();
        for (const auto& item : dataSet.items) {
            json itemJson;
            itemJson["name"] = item.name;
            itemJson["type"] = item.type;

            // 尝试解析值为JSON
            try {
                json valueJson = json::parse(item.value);
                itemJson["value"] = valueJson;
            }
            catch (...) {
                // 如果不是JSON格式，作为字符串存储
                itemJson["value"] = item.value;
            }

            itemJson["description"] = item.description;
            itemsJson.push_back(itemJson);
        }

        j["items"] = itemsJson;

        // 保存到文件
        std::ofstream file(filePath);
        if (!file.is_open()) {
            throw TestDataException("Failed to create file: " + filePath);
        }

        file << std::setw(4) << j << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        throw TestDataException("Failed to export DataSet: " + std::string(e.what()));
    }
}
