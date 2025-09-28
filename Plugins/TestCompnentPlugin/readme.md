MSVC 工程配置（TestAutomation.vcxproj）
需与 AutomationCore 的工程配置对齐，避免编译 / 链接错误，关键配置步骤如下：
1. 常规配置
配置类型：动态库(.dll)（工程 → 右键 → 属性 → 常规 → 配置类型）。
平台：与 AutomationCore 一致（建议 x64，避免 32/64 位不兼容）。
字符集：使用多字节字符集（Win32 API 推荐，与核心框架保持一致）。
2. C/C++ 配置
附加包含目录：添加 ../../AutomationCore（工程 → 属性 → C/C++ → 常规 → 附加包含目录），确保能找到 PluginManager.h、TestData.h 等核心头文件。
预处理器定义：添加 TESTAUTOMATION_EXPORTS;_WINDOWS;_USRDLL（工程 → 属性 → C/C++ → 预处理器 → 预处理器定义），其中 TESTAUTOMATION_EXPORTS 确保导出宏生效。
警告等级：等级3 (/W3)，与核心框架保持一致，避免不必要的警告。
3. 链接器配置
附加库目录：添加 ../../AutomationCore/x64/Debug（或 Release，根据编译模式选择），确保能找到 AutomationCore.lib（核心框架的导入库）。
附加依赖项：添加 AutomationCore.lib;user32.lib（工程 → 属性 → 链接器 → 输入 → 附加依赖项），其中：
AutomationCore.lib：依赖核心框架的导出函数。
user32.lib：Win32 API 依赖库（窗口控制、消息发送等）。
生成导入库：勾选 生成导入库（工程 → 属性 → 链接器 → 高级 → 生成导入库），确保核心框架能链接插件。
四、插件集成与测试
编译插件：生成 TestAutomation.dll 和 TestAutomation.lib（输出目录建议与 AutomationCore 的插件目录一致，如 AutomationCore/Plugins）。
核心框架加载：AutomationCore 的 PluginManager 通过 LoadLibrary 加载 TestAutomation.dll，调用 CreatePluginInstance 获取插件实例。
UI 配置：在 AutomationUI 中选择 Win32AutomationPlugin，配置测试步骤（如选择 Win32_Click，输入 WindowHandle 和 ControlId）。
执行测试：核心框架调度插件执行步骤，结果通过 TestResult 返回，存入 SQLite 并在 UI 展示。
五、关键注意事项
名称修饰问题：CreatePluginInstance 必须用 extern "C" 声明，避免 MSVC 对函数名进行 C++ 名称修饰，导致核心框架无法找到该函数。
参数兼容性：插件的 ExecuteTestStep 接收的 TestStep 结构，必须与 AutomationCore 定义的完全一致（字段名、类型、顺序）。
资源释放：Win32 句柄（如 HWND）无需手动 CloseHandle，由系统管理，插件仅需清理缓存的句柄变量。
日志规范：所有操作日志通过 Log::WriteLog 输出，便于核心框架统一管理日志（如写入文件、控制台）。
通过以上代码和配置，Win32 自动化插件可无缝集成到现有系统中，支持窗口点击、文本设置、标题获取等核心自动化功能，后续可根据需求扩展更多 Win32 操作（如菜单选择、键盘模拟等）。