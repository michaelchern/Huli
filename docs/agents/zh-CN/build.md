# Huli Build Context

只在 CMake、构建、Visual Studio、验证命令或编译错误任务中加载本文件。

## 当前状态

- Huli 目前是学习仓库骨架。
- 根 `CMakeLists.txt` 目前只声明最低 CMake 版本，没有 `project()` 或可构建目标。
- 当前没有约定源码目录、shader 目录或运行入口；新增后再更新本文件。

## 规则

- 修改前运行 `git status --short --branch`。
- 先判断失败来自 Huli 构建骨架、MSVC/SDK 环境还是 Vulkan SDK。
- 如果 Huli 尚未定义目标，明确报告“当前仓库未配置完整构建目标”，不要把它解释成 Vulkan 逻辑失败。
- C++ / CMake / shader 改动后，优先运行最小相关命令；纯文档改动不需要 CMake 构建。

## 参考命令

```powershell
git status --short --branch
if (Test-Path CMakePresets.json) { cmake --list-presets }
cmake -S . -B build
cmake --build build
```

如果 MSVC 环境缺失，优先使用 Visual Studio Developer Command Prompt 或显式加载 `VsDevCmd.bat` 后重试。
