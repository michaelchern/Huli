# Huli Build Smoke

## 当前事实

- 根 `CMakeLists.txt` 可以配置并构建第三方依赖。
- Huli 应用目标和 `src/vulkan` 尚未接入根构建，因此依赖构建成功不等于应用编译或 Vulkan 运行验证成功。
- 当前依赖版本与条件以实时 `CMakeLists.txt` 为准；本文件只保存验证入口和带日期的证据。

## 2026-07-11 验证快照

- `out/build/codex-dependency-check-20260711/CMakeCache.txt` 记录 CMake 4.3.1、MSVC 19.51、Ninja、Debug 和 Vulkan SDK `1.4.350.0`。
- `ninja -C out/build/codex-dependency-check-20260711 -n` 返回 `ninja: no work to do.`，说明该依赖构建目录在快照时没有待执行任务。
- `out/build/x64-Debug/CMakeCache.txt` 仍记录旧 Vulkan SDK `1.4.341.1`；通过 Visual Studio 使用 `x64-Debug` 前应 Delete Cache and Reconfigure，不能把旧缓存当成当前环境证据。

## 全新依赖检查

在已经加载 MSVC 开发环境的 PowerShell 中运行：

```powershell
git status --short --branch
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake -S . -B out/build/dependency-check -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DBUILD_TESTING=OFF `
  -DGLI_TEST_ENABLE=OFF `
  -DTRACY_CALLSTACK=OFF `
  -DTRACY_ENABLE=OFF `
  -DIMGUI_EXAMPLES=OFF

cmake --build out/build/dependency-check --parallel 8
cmake --build out/build/dependency-check --parallel 8
```

验收标准：

- 配置日志包含 `Configuring done` 和 `Generating done`。
- 第一次完整构建退出代码为 `0`。
- 未修改输入时第二次构建显示 `ninja: no work to do.`。
- `CMakeCache.txt` 中的编译器、生成器和 Vulkan SDK 与预期一致。

## Visual Studio 检查

1. 确认 `CMakeSettings.json` 可以解析。
2. 修改 Vulkan SDK 环境后重启 Visual Studio。
3. 对目标配置执行 Delete Cache and Reconfigure。
4. 检查新生成的 `out/build/<配置>/CMakeCache.txt`，不要只看系统环境变量。

```powershell
Get-Content CMakeSettings.json -Raw | ConvertFrom-Json | Out-Null
rg -n "^(CMAKE_GENERATOR|CMAKE_BUILD_TYPE|CMAKE_CXX_COMPILER|Vulkan_INCLUDE_DIR|Vulkan_LIBRARY):" `
  out/build/x64-Debug/CMakeCache.txt
```

## 判断方式

- MSVC 标准头、Windows SDK 或 Vulkan SDK 找不到时，先修复开发环境或清理旧缓存。
- 依赖下载长时间无日志时，先检查 `git`/CMake 子进程，再判断是否卡死。
- 只有 Huli 应用目标接入后，才增加对应目标的编译和运行 smoke；在此之前不要报告 Vulkan runtime 已通过。
- 外部教材或参考仓库不属于固定 smoke 入口；只有用户明确指定时才单独记录来源和验证目的。

## 文档验证

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```
