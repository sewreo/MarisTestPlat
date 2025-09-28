#ifndef PLUGIN_EXPORT_H
#define PLUGIN_EXPORT_H

// 遵循现有代码的编译宏命名风格，TESTAUTOMATION_EXPORTS 由 MSVC 工程定义（插件编译时导出）
#ifdef TESTAUTOMATION_EXPORTS
#define TESTAUTOMATION_API __declspec(dllexport)
#else
#define TESTAUTOMATION_API __declspec(dllimport)
#endif

#endif // PLUGIN_EXPORT_H