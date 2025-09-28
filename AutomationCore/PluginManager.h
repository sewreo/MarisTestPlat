#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <memory>

#include "IPluginManager.h"


/**
 * @brief 插件管理器类，负责加载、卸载和管理所有插件
 */
class PluginManager:public IPluginManager
{
public:
    PluginManager() = default;
    ~PluginManager();

    // 禁止拷贝和移动
    PluginManager(const PluginManager &) = delete;
    PluginManager &operator=(const PluginManager &) = delete;
    PluginManager(PluginManager &&) = delete;
    PluginManager &operator=(PluginManager &&) = delete;

    /**
     * @brief 加载单个插件
     * @param dllPath 插件DLL路径
     * @return 加载是否成功
     */
    bool loadPlugin(const std::string &dllPath) override;

    /**
     * @brief 加载指定目录下的所有插件
     * @param pluginDir 插件目录
     * @return 成功加载的插件数量
     */
    int loadPluginsFromDirectory(const std::string &pluginDir) override;

    /**
     * @brief 卸载指定名称的插件
     * @param pluginName 插件名称
     * @return 卸载是否成功
     */
    bool unloadPlugin(const std::string &pluginName) override;

    /**
     * @brief 卸载所有插件
     */
    void unloadAllPlugins() override;

    /**
     * @brief 获取所有加载的插件
     * @return 插件指针列表
     */
    std::vector<IAutomationPlugin *> getPlugins() const override;

    /**
     * @brief 根据名称获取插件
     * @param pluginName 插件名称
     * @return 插件指针，不存在则返回nullptr
     */
    IAutomationPlugin * getPlugin(const std::string &pluginName) const override;

    /**
     * @brief 获取插件信息列表
     * @return 插件名称和版本的映射
     */
    std::unordered_map<std::string, std::string> getPluginInfos() const override;

    /**
 * @brief 检查插件是否已加载
 * @param pluginName 插件名称
 * @return 已加载返回true，否则返回false
 */
    bool hasPlugin(const std::string& pluginName) const override;

private:
    /**
     * @brief 检查插件是否实现了必要的接口
     * @param plugin 插件实例
     * @return 是否有效
     */
    bool validatePlugin(IAutomationPlugin *plugin) const;

    std::unordered_map<std::string, PluginHandle> plugins_; // 插件名称到插件句柄的映射
};
