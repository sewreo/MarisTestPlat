#include "Win32Plugin.h"
#include <algorithm>
#include <sstream>
#include <chrono>

// -------------------------- 1. 插件基本信息实现 --------------------------
std::string Win32Plugin::name() const
{
    return "Win32AutomationPlugin"; // 唯一名称，与其他插件区分
}

std::string Win32Plugin::version() const
{
    return "1.0.0"; // 语义化版本：主版本.次版本.修订版本
}

std::vector<std::string> Win32Plugin::supportedActions() const
{
    // 返回插件支持的所有操作类型（UI 配置步骤时会展示这些选项）
    return {
        "Win32_Click",         // 模拟按钮点击
        "Win32_SetText",       // 设置文本框内容
        "Win32_GetWindowTitle" // 获取窗口标题
    };
}

// -------------------------- 2. 插件生命周期实现 --------------------------
bool Win32Plugin::initialize(/*const std::map<std::string, std::string>& config*/)
{
    if (m_isInitialized)
    {
        LOG_INFO(name()  + " Plugin already initialized");
        return true;
    }

    /*
    // 从配置中读取默认超时时间（若未配置，使用默认值 5000ms）
    auto timeoutIt = config.find("DefaultTimeout");
    if (timeoutIt != config.end())
    {
        try
        {
            m_defaultTimeout = std::stoi(timeoutIt->second);
            LOG_INFO(name() + "Loaded default timeout: " + std::to_string(m_defaultTimeout) + "ms");
        }
        catch (const std::invalid_argument &e)
        {
            LOG_ERROR(name() + "Invalid DefaultTimeout value: " + std::string(e.what()));
            return false;
        }
    }
    else
    {
        LOG_INFO(name() + "DefaultTimeout not configured, use default: 5000ms");
    }

    // 初始化目标窗口句柄（若配置中提供，提前缓存）
    auto hwndIt = config.find("TargetWindowHandle");
    if (hwndIt != config.end())
    {
        try
        {
            m_targetWindow = (HWND)std::stoull(hwndIt->second, nullptr, 16); // 16进制字符串转句柄
            if (!IsWindowValid(m_targetWindow))
            {
                Log::WriteLog(LogLevel::WARNING, "Win32Plugin", "Configured TargetWindowHandle is invalid, reset to nullptr");
                m_targetWindow = nullptr;
            }
            else
            {
                LOG_INFO(name() + "Loaded target window handle: " + hwndIt->second);
            }
        }
        catch (const std::invalid_argument &e)
        {
            LOG_ERROR(name() + "Invalid TargetWindowHandle: " + std::string(e.what()));
            return false;
        }
    }
    */
    m_isInitialized = true;
    LOG_INFO(name() + " Initialize success");
    return true;
}

void Win32Plugin::uninitialize()
{
    if (!m_isInitialized)
    {
        LOG_INFO(name() + "Plugin already uninitialized");
        return;
    }

    // 释放 Win32 资源（句柄无需手动关闭，由系统管理，此处仅清理缓存）
    m_targetWindow = nullptr;
    m_isInitialized = false;

    LOG_INFO(name() + "Uninitialize success");
}

// -------------------------- 3. 核心功能：执行测试步骤 --------------------------
StepResult Win32Plugin::executeStep(const StepParam& param)
//TestResult Win32Plugin::ExecuteTestStep(const TestStep &step)
{
    StepResult result;
    result.success = false;                         // 默认失败，执行成功后修改
    result.action = param.action;                                // 关联测试步骤 ID，便于结果匹配
    auto startTime = std::chrono::high_resolution_clock::now(); // 记录执行时间

    // 检查插件初始化状态
    if (!m_isInitialized)
    {
        result.message = "Plugin not initialized";
        result.err_info = "Call Initialize() before executing steps";
        LOG_ERROR(name() + "Step " + param.action + ": " + result.message);
        return result;
    }

    // 检查操作类型是否支持
    auto supportedOps = supportedActions();
    if (std::find(supportedOps.begin(), supportedOps.end(), param.target) == supportedOps.end())
    {
        result.message = "Unsupported operation";
        result.err_info = "Operation: " + param.target + ", supported: " + [&]()
        {
            std::string opsStr;
            for (const auto& op : supportedOps)
                opsStr += op + ", ";
            return opsStr.substr(0, opsStr.size() - 2);
        }();
        LOG_ERROR(name() + "Step " + param.action + ": " + result.message);
        return result;
    }

    // 根据操作类型分发到具体实现
    if (param.target == "Win32_Click")
    {
        result = SimulateButtonClick(param.params);
    }
    else if (param.target == "Win32_SetText")
    {
        result = SetTextBoxContent(param.params);
    }
    else if (param.target == "Win32_GetWindowTitle")
    {
        result = GetWindowTitle(param.params);
    }

    // 计算执行时间（毫秒）
    auto endTime = std::chrono::high_resolution_clock::now();
    result.ExecutionTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    // 输出执行日志
    std::string logMsg = "Step " + param.action + ": " + (result.success? "Success" : "Failed");

    if (result.success)
    {
        LOG_INFO(name() + logMsg);
    }
    else
    {
        LOG_ERROR(name() + logMsg);
    }


    return result;
}

// -------------------------- 内部辅助函数：Win32 API 实现 --------------------------
StepResult Win32Plugin::SimulateButtonClick(const std::map<std::string, std::string> &params)
{
    StepResult result;
    result.success = false;

    // 1. 检查必要参数（WindowHandle、ControlId）
    std::vector<std::string> requiredParams = {"WindowHandle", "ControlId"};
    for (const auto &param : requiredParams)
    {
        if (params.find(param) == params.end() || params.at(param).empty())
        {
            result.message = "Missing required parameter";
            result.err_info = "Parameter: " + param;
            return result;
        }
    }

    // 2. 转换参数（字符串 → Win32 类型）
    HWND hwnd = nullptr;
    int controlId = 0;
    try
    {
        // 窗口句柄：16进制字符串（如 "0x12345678"）转 HWND
        hwnd = (HWND)std::stoull(params.at("WindowHandle"), nullptr, 16);
        // 控件 ID：字符串转整数
        controlId = std::stoi(params.at("ControlId"));
    }
    catch (const std::invalid_argument &e)
    {
        result.message = "Invalid parameter format";
        result.err_info = "Error: " + std::string(e.what());
        return result;
    }

    // 3. 检查窗口和控件有效性
    if (!IsWindowValid(hwnd))
    {
        result.message = "Invalid window handle";
        result.err_info = "WindowHandle: " + params.at("WindowHandle");
        return result;
    }

    HWND hControl = GetDlgItem(hwnd, controlId); // 获取子控件句柄
    if (hControl == nullptr)
    {
        result.message = "Control not found";
        result.err_info = "ControlId: " + params.at("ControlId") + ", WindowHandle: " + params.at("WindowHandle");
        return result;
    }

    // 4. 模拟按钮点击（Win32 API：发送 BM_CLICK 消息）
    BOOL clickResult = SendMessage(hControl, BM_CLICK, 0, 0);
    if (clickResult == 0)
    {
        result.message = "Click operation failed";
        result.err_info = "SendMessage(BM_CLICK) returned 0, ErrorCode: " + std::to_string(GetLastError());
        return result;
    }

    // 5. 执行成功
    result.success = true;
    result.message = "Button click success";
    result.extra_data = params.at("ControlId"); // 输出关键参数，便于后续分析
    return result;
}

StepResult Win32Plugin::SetTextBoxContent(const std::map<std::string, std::string> &params)
{
    StepResult result;
    result.success = false;

    // 1. 检查必要参数（WindowHandle、ControlId、Text）
    std::vector<std::string> requiredParams = {"WindowHandle", "ControlId", "Text"};
    for (const auto &param : requiredParams)
    {
        if (params.find(param) == params.end() || params.at(param).empty())
        {
            result.message = "Missing required parameter";
            result.err_info = "Parameter: " + param;
            return result;
        }
    }

    // 2. 转换参数
    HWND hwnd = nullptr;
    int controlId = 0;
    std::string text = params.at("Text");
    try
    {
        hwnd = (HWND)std::stoull(params.at("WindowHandle"), nullptr, 16);
        controlId = std::stoi(params.at("ControlId"));
    }
    catch (const std::invalid_argument &e)
    {
        result.message = "Invalid parameter format";
        result.err_info = "Error: " + std::string(e.what());
        return result;
    }

    // 3. 检查窗口和控件有效性
    if (!IsWindowValid(hwnd))
    {
        result.message = "Invalid window handle";
        result.err_info = "WindowHandle: " + params.at("WindowHandle");
        return result;
    }

    HWND hControl = GetDlgItem(hwnd, controlId);
    if (hControl == nullptr)
    {
        result.message = "Control not found";
        result.err_info = "ControlId: " + params.at("ControlId");
        return result;
    }

    // 4. 检查控件是否为文本框（通过类名验证：EDIT 是标准文本框类名）
    char className[256] = {0};
    GetClassName(hControl, className, sizeof(className) - 1);
    if (std::string(className) != "EDIT")
    {
        result.message = "Control is not a text box";
        result.err_info = "Control class name: " + std::string(className);
        return result;
    }

    // 5. 设置文本框内容（Win32 API：SetWindowText）
    BOOL setResult = SetWindowText(hControl, text.c_str());
    if (!setResult)
    {
        result.message = "Set text failed";
        result.err_info = "ErrorCode: " + std::to_string(GetLastError());
        return result;
    }

    // 6. 执行成功
    result.success = true;
    result.message = "Set text success";
    result.extra_data = text; // 输出设置的文本，便于结果验证
    return result;
}

StepResult Win32Plugin::GetWindowTitle(const std::map<std::string, std::string> &params)
{
    StepResult result;
    result.success = false;

    // 1. 检查必要参数（WindowHandle）
    if (params.find("WindowHandle") == params.end() || params.at("WindowHandle").empty())
    {
        result.message = "Missing required parameter";
        result.err_info = "Parameter: WindowHandle";
        return result;
    }

    // 2. 转换参数
    HWND hwnd = nullptr;
    try
    {
        hwnd = (HWND)std::stoull(params.at("WindowHandle"), nullptr, 16);
    }
    catch (const std::invalid_argument &e)
    {
        result.message = "Invalid WindowHandle format";
        result.err_info = "Error: " + std::string(e.what());
        return result;
    }

    // 3. 检查窗口有效性
    if (!IsWindowValid(hwnd))
    {
        result.message = "Invalid window handle";
        result.err_info = "WindowHandle: " + params.at("WindowHandle");
        return result;
    }

    // 4. 获取窗口标题（Win32 API：GetWindowText）
    char windowTitle[512] = {0};
    int titleLen = GetWindowText(hwnd, windowTitle, sizeof(windowTitle) - 1);
    if (titleLen == 0)
    {
        // 检查是否为错误（GetLastError() 为 0 表示窗口无标题，非错误）
        if (GetLastError() != 0)
        {
            result.message = "Get window title failed";
            result.err_info = "ErrorCode: " + std::to_string(GetLastError());
            return result;
        }
        else
        {
            windowTitle[0] = '\0'; // 无标题时设为空字符串
        }
    }

    // 5. 执行成功（标题存入 OutputData，供 UI 展示或断言）
    result.success = true;
    result.message = "Get window title success";
    result.extra_data = std::string(windowTitle);
    return result;
}

bool Win32Plugin::IsWindowValid(HWND hwnd) const
{
    // 检查句柄是否为空 + 窗口是否存在 + 窗口是否可见（可选，根据需求调整）
    return hwnd != nullptr && IsWindow(hwnd) && IsWindowVisible(hwnd);
}

// -------------------------- 插件实例创建函数（核心框架调用入口） --------------------------
extern "C" TESTAUTOMATION_API IAutomationPlugin *CreatePluginInstance()
{
    // 返回插件实例（核心框架通过此函数加载插件，必须为 C 链接，避免名称修饰）
    return new Win32Plugin();
}