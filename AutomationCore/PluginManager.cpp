#include "PluginManager.h"
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

namespace fs = std::filesystem;

PluginManager::~PluginManager() {
    unloadAllPlugins();
}

bool PluginManager::loadPlugin(const std::string& dllPath) {
    // 检查文件是否存在
    if (!PathFileExistsA(dllPath.c_str())) {
        std::cerr << "Plugin file not found: " << dllPath << std::endl;
        return false;
    }

    // 加载DLL
    HMODULE hModule = LoadLibraryA(dllPath.c_str());
    if (!hModule) {
        std::cerr << "Failed to load plugin: " << dllPath
            << ", Error: " << GetLastError() << std::endl;
        return false;
    }

    // 获取创建和销毁函数
    CreatePluginFunc createFunc = reinterpret_cast<CreatePluginFunc>(
        GetProcAddress(hModule, "createPlugin")
        );

    DestroyPluginFunc destroyFunc = reinterpret_cast<DestroyPluginFunc>(
        GetProcAddress(hModule, "destroyPlugin")
        );

    if (!createFunc || !destroyFunc) {
        std::cerr << "Plugin " << dllPath << " does not implement required functions" << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // 创建插件实例
    IAutomationPlugin* plugin = createFunc();
    if (!plugin) {
        std::cerr << "Failed to create plugin instance from: " << dllPath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // 验证插件
    if (!validatePlugin(plugin)) {
        std::cerr << "Plugin " << plugin->name() << " is invalid" << std::endl;
        destroyFunc(plugin);
        FreeLibrary(hModule);
        return false;
    }

    // 检查是否已存在同名插件
    std::string pluginName = plugin->name();
    if (plugins_.find(pluginName) != plugins_.end()) {
        std::cerr << "Plugin " << pluginName << " already loaded" << std::endl;
        destroyFunc(plugin);
        FreeLibrary(hModule);
        return false;
    }

    // 初始化插件
    if (!plugin->initialize()) {
        std::cerr << "Failed to initialize plugin: " << pluginName << std::endl;
        destroyFunc(plugin);
        FreeLibrary(hModule);
        return false;
    }

    // 存储插件句柄
    PluginHandle handle;
    handle.hModule = hModule;
    handle.plugin.reset(plugin);
    handle.createFunc = createFunc;
    handle.destroyFunc = destroyFunc;

    plugins_[pluginName] = std::move(handle);
    std::cout << "Successfully loaded plugin: " << pluginName << " (" << plugin->version() << ")" << std::endl;

    return true;
}

int PluginManager::loadPluginsFromDirectory(const std::string& pluginDir) {
    if (!fs::exists(pluginDir) || !fs::is_directory(pluginDir)) {
        std::cerr << "Plugin directory does not exist: " << pluginDir << std::endl;
        return 0;
    }

    int loadedCount = 0;

    // 遍历目录中的所有DLL文件
    for (const auto& entry : fs::directory_iterator(pluginDir)) {
        if (entry.is_regular_file() &&
            entry.path().extension() == ".dll") {
            if (loadPlugin(entry.path().string())) {
                loadedCount++;
            }
        }
    }

    return loadedCount;
}

bool PluginManager::unloadPlugin(const std::string& pluginName) {
    auto it = plugins_.find(pluginName);
    if (it == plugins_.end()) {
        return false;
    }

    // 反初始化插件
    it->second.plugin->uninitialize();

    // 调用销毁函数
    it->second.destroyFunc(it->second.plugin.release());

    // 释放DLL
    FreeLibrary(it->second.hModule);

    // 从映射中移除
    plugins_.erase(it);

    std::cout << "Unloaded plugin: " << pluginName << std::endl;
    return true;
}

void PluginManager::unloadAllPlugins() {
    for (auto& pair : plugins_) {
        // 反初始化插件
        pair.second.plugin->uninitialize();

        // 调用销毁函数
        pair.second.destroyFunc(pair.second.plugin.release());

        // 释放DLL
        FreeLibrary(pair.second.hModule);
    }

    plugins_.clear();
    std::cout << "All plugins unloaded" << std::endl;
}

std::vector<IAutomationPlugin*> PluginManager::getPlugins() const {
    std::vector<IAutomationPlugin*> result;
    result.reserve(plugins_.size());

    for (const auto& pair : plugins_) {
        result.push_back(pair.second.plugin.get());
    }

    return result;
}

IAutomationPlugin* PluginManager::getPlugin(const std::string& pluginName) const {
    auto it = plugins_.find(pluginName);
    if (it != plugins_.end()) {
        return it->second.plugin.get();
    }
    return nullptr;
}

std::unordered_map<std::string, std::string> PluginManager::getPluginInfos() const {
    std::unordered_map<std::string, std::string> infos;

    for (const auto& pair : plugins_) {
        const auto& plugin = pair.second.plugin;
        infos[plugin->name()] = plugin->version();
    }

    return infos;
}


bool PluginManager::hasPlugin(const std::string& pluginName) const {
    return plugins_.find(pluginName) != plugins_.end();
}

bool PluginManager::validatePlugin(IAutomationPlugin* plugin) const {
    if (!plugin) return false;

    // 检查插件名称是否有效
    if (plugin->name().empty()) {
        std::cerr << "Plugin has empty name" << std::endl;
        return false;
    }

    // 检查版本信息
    if (plugin->version().empty()) {
        std::cerr << "Plugin " << plugin->name() << " has no version info" << std::endl;
        // 版本信息缺失视为警告，不影响加载
    }

    return true;
}
