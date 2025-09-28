#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

// 前向声明
struct StepParam;
struct StepResult;

/**
 * @brief 自动化步骤参数结构
 */
struct StepParam {
    std::string action;       // 动作名称(如: click, input, check)
    std::string target;       // 目标控件标识(如: 窗口标题, 控件ID)
    std::string value;        // 操作值(如: 输入文本, 等待时间)
    std::map<std::string, std::string> params;
    int timeout = 3000;       // 超时时间(毫秒)
};

/**
 * @brief 自动化步骤执行结果
 */
struct StepResult {
    bool success = false;     // 是否成功
    std::string message;      // 结果消息
    int error_code = 0;       // 错误代码(0表示无错误)
    std::string extra_data;   // 额外数据(如: 获取的控件文本)
    std::string err_info;
    std::string action;
    long long ExecutionTimeMs;
};



enum class TestStatus
{
    FAILED = 0,
    PASS = 1
};

/**
 * @brief 自动化插件接口类，所有插件必须实现此接口
 */
class IAutomationPlugin {
public:
    virtual ~IAutomationPlugin() = default;

    /**
     * @brief 获取插件名称
     * @return 插件名称
     */
    virtual std::string name() const = 0;

    /**
     * @brief 获取插件描述
     * @return 插件描述
     */
    virtual std::string description() const = 0;

    /**
     * @brief 获取插件版本
     * @return 版本字符串
     */
    virtual std::string version() const = 0;

    /**
     * @brief 初始化插件
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 反初始化插件
     */
    virtual void uninitialize() = 0;

    /**
     * @brief 执行单个自动化步骤
     * @param param 步骤参数
     * @return 执行结果
     */
    virtual StepResult executeStep(const StepParam& param) = 0;

    /**
     * @brief 获取支持的动作列表
     * @return 动作名称列表
     */
    virtual std::vector<std::string> supportedActions() const = 0;
};

// 插件创建和销毁函数类型定义
typedef IAutomationPlugin* (*CreatePluginFunc)();
typedef void (*DestroyPluginFunc)(IAutomationPlugin*);

// 插件接口ID和导出宏
#define AUTOMATION_PLUGIN_INTERFACE_ID "Automation.IAutomationPlugin/1.0"
#define DECLARE_AUTOMATION_PLUGIN(PluginClass) \
    extern "C" __declspec(dllexport) IAutomationPlugin* createPlugin() { \
        return new PluginClass(); \
    } \
    extern "C" __declspec(dllexport) void destroyPlugin(IAutomationPlugin* plugin) { \
        delete plugin; \
    }
