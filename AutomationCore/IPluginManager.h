#pragma once

#include <string>
#include <vector>
#include <memory>
#include <windows.h>
#include "IAutomationPlugin.h"
#include <unordered_map>

/**
 * @brief 插件句柄结构，管理插件的模块和实例
 */
struct PluginHandle
{
    HMODULE hModule = nullptr;                      // 模块句柄
    std::unique_ptr<IAutomationPlugin> plugin = nullptr; // 插件实例
    std::string name;                          // 插件名称
    std::string version;                       // 插件版本
    CreatePluginFunc createFunc = nullptr;     // 创建函数
    DestroyPluginFunc destroyFunc = nullptr;   // 销毁函数
};


/**
 * @brief 插件管理器接口
 * 负责插件的加载、卸载和管理
 */
class IPluginManager
{
public:
    /**
     * @brief 析构函数
     */
    virtual ~IPluginManager() = default;

    /**
     * @brief 从目录加载所有插件
     * @param directory 插件目录路径
     * @return 成功加载的插件数量
     */
    virtual int loadPluginsFromDirectory(const std::string &directory) = 0;

    /**
     * @brief 加载单个插件
     * @param dllPath 插件DLL路径
     * @return 加载成功返回true，否则返回false
     */
    virtual bool loadPlugin(const std::string &dllPath) = 0;

    /**
     * @brief 卸载所有插件
     */
    virtual void unloadAllPlugins() = 0;

    /**
     * @brief 卸载指定插件
     * @param pluginName 插件名称
     * @return 卸载成功返回true，否则返回false
     */
    virtual bool unloadPlugin(const std::string &pluginName) = 0;

    /**
     * @brief 获取所有加载的插件
     * @return 插件指针列表
     */
    virtual std::vector<IAutomationPlugin *> getPlugins() const = 0;

    /**
     * @brief 获取指定名称的插件
     * @param pluginName 插件名称
     * @return 插件指针，不存在返回nullptr
     */
    virtual IAutomationPlugin *getPlugin(const std::string &pluginName) const = 0;

    /**
     * @brief 检查插件是否已加载
     * @param pluginName 插件名称
     * @return 已加载返回true，否则返回false
     */
    virtual bool hasPlugin(const std::string &pluginName) const = 0;


    /**
     * @brief 获取插件信息列表
     * @return 插件名称和版本的映射
     */
    virtual std::unordered_map<std::string, std::string> getPluginInfos() const = 0;

};
