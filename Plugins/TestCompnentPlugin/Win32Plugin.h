#ifndef WIN32_PLUGIN_H
#define WIN32_PLUGIN_H

#include "../PluginExport.h"
// 引入核心框架头文件（路径与仓库目录结构匹配）
#include "../../AutomationCore/IPluginManager.h"
#include "../../AutomationCore/ITestDataManager.h"      // 包含 TestStep、TestResult 结构
#include "../../AutomationCore/Logger.h"           // 包含核心日志工具
#include "../../AutomationCore/IAutomationPlugin.h"

#include <string>
#include <map>
#include <vector>
#include <Windows.h> // Win32 API 头文件

// 继承核心框架的插件接口，实现 Win32 自动化功能
class TESTAUTOMATION_API Win32Plugin : public IAutomationPlugin
{
public:
    Win32Plugin() = default;
    ~Win32Plugin() override = default;

    // -------------------------- 1. 实现 IAutomationPlugin 纯虚函数（插件基本信息） --------------------------
    /**
     * @brief 获取插件名称（唯一标识，供 UI 选择和 Core 管理）
     */
    std::string name() const override;

    /**
     * @brief 获取插件版本（遵循语义化版本）
     */
    std::string version() const override;

    /**
     * @brief 获取插件支持的操作列表（供 UI 展示可配置的步骤类型）
     */
    std::vector<std::string> supportedActions() const override;

    // -------------------------- 2. 实现 IAutomationPlugin 纯虚函数（插件生命周期） --------------------------
    /**
     * @brief 初始化插件（加载 Win32 资源、初始化状态）
     * @param config 插件配置参数（如默认超时、目标窗口信息）
     * @return 初始化成功返回 true，失败返回 false
     */
   // bool Initialize(const std::map<std::string, std::string> &config) override;
    bool initialize() override;

    /**
     * @brief 卸载插件（释放 Win32 句柄、清理资源）
     */
    void uninitialize() override;

    // -------------------------- 3. 实现 IAutomationPlugin 纯虚函数（核心功能：执行测试步骤） --------------------------
    /**
     * @brief 执行 Win32 自动化测试步骤
     * @param step 测试步骤（含操作类型、参数，如窗口句柄、控件 ID）
     * @return 测试结果（状态、消息、错误信息）
     */
    StepResult executeStep(const StepParam& param) override;
   // TestResult ExecuteTestStep(const TestStep &step) override;

private:
    // -------------------------- 内部辅助函数（Win32 API 封装） --------------------------
    /**
     * @brief 模拟按钮点击（对应操作："Win32_Click"）
     * @param params 操作参数：WindowHandle（窗口句柄）、ControlId（控件 ID）
     * @return 操作结果
     */
    StepResult SimulateButtonClick(const std::map<std::string, std::string> &params);

    /**
     * @brief 设置文本框内容（对应操作："Win32_SetText"）
     * @param params 操作参数：WindowHandle、ControlId、Text（待设置文本）
     * @return 操作结果
     */
    StepResult SetTextBoxContent(const std::map<std::string, std::string> &params);

    /**
     * @brief 获取窗口标题（对应操作："Win32_GetWindowTitle"）
     * @param params 操作参数：WindowHandle（窗口句柄）
     * @return 操作结果（标题存入结果的 OutputData 字段）
     */
    StepResult GetWindowTitle(const std::map<std::string, std::string> &params);

    /**
     * @brief 检查窗口是否有效（辅助函数）
     * @param hwnd 窗口句柄
     * @return 有效返回 true，无效返回 false
     */
    bool IsWindowValid(HWND hwnd) const;

    // -------------------------- 插件状态变量 --------------------------
    bool m_isInitialized = false;  // 插件初始化状态（避免重复初始化/卸载）
    HWND m_targetWindow = nullptr; // 目标窗口句柄缓存（优化性能，避免重复查找）
    int m_defaultTimeout = 5000;   // 默认超时时间（毫秒，从配置中读取）
};

// -------------------------- 插件实例创建函数（核心框架依赖！必须按此签名实现） --------------------------
/**
 * @brief 供 AutomationCore 调用，创建插件实例
 * @return 插件实例指针（IAutomationPlugin 接口类型）
 */
extern "C" TESTAUTOMATION_API IAutomationPlugin *CreatePluginInstance();

#endif // WIN32_PLUGIN_H