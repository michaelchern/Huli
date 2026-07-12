# Huli Build Smoke

## 当前事实

- 根 `CMakeLists.txt` 可以配置并构建第三方依赖与 `src/vulkan` 的 `huli_vulkan` 静态库。
- Huli 应用目标和 Vulkan 运行入口尚未接入；`huli_vulkan` 编译成功不等于应用编译或 Vulkan 运行验证成功。
- 当前依赖版本与条件以实时 `CMakeLists.txt` 为准；本文件只保存验证入口和带日期的证据。

## 2026-07-11 验证快照

- `out/build/codex-dependency-check-20260711/CMakeCache.txt` 记录 CMake 4.3.1、MSVC 19.51、Ninja、Debug 和 Vulkan SDK `1.4.350.0`。
- `ninja -C out/build/codex-dependency-check-20260711 -n` 返回 `ninja: no work to do.`，说明该依赖构建目录在快照时没有待执行任务。

## 2026-07-12 Presets 验证快照

- `cmake --list-presets` 枚举 4 个 configure presets，`cmake --build --list-presets` 枚举 8 个 build presets。
- `ninja-msvc` 与 `ninja-clang` 全新配置成功；缓存分别记录 MSVC 19.51 与 Clang 22.1.1 GNU 风格驱动、Ninja Multi-Config 和 Vulkan SDK `1.4.350.0`。
- 在 `src/vulkan` 接入前，`msvc-debug` 与 `clang-debug` 均完成当时的根依赖构建，退出代码为 `0`；MSVC 未修改后再次构建返回 `ninja: no work to do.`。
- `msvc-release`、`msvc-relwithdebinfo`、`clang-release`、`clang-relwithdebinfo` 均能选择同一工具链构建树中的正确 configuration ninja 文件。
- `msvc-asan-relwithdebinfo` 完整构建通过，缓存与详细编译命令包含 `/fsanitize=address`，且没有 Debug 的 `/RTC1`。
- `msvc-tracy-release` 完整构建通过，缓存记录 `TRACY_ENABLE=ON` 和 `TRACY_CALLSTACK=ON`。
- 当前根项目未实际消费 preset 中的 `BUILD_TESTING`、`GLI_TEST_ENABLE`、`IMGUI_EXAMPLES`，配置会报告 manually-specified variables unused warning；该 warning 非构建失败，但在对应根选项真正接入前不能声称这些开关产生了行为变化。

## 2026-07-12 Vulkan 模块接入快照

- 根 CMake 已通过 `add_subdirectory(src/vulkan)` 接入 `huli_vulkan`，并提供 `Huli::Vulkan` 别名。
- `cmake --build --preset msvc-debug --target huli_vulkan --parallel 1` 与对应 Clang Debug 目标构建均退出 `0`，确认 `src/vulkan` 源码实际参与编译。
- 临时下游 executable 只链接 `Huli::Vulkan`，包含 `<vulkan/Context.hpp>`，并引用 Huli Vulkan 符号与 `GetDefaultResources()`；MSVC 编译、链接和运行均退出 `0`。
- 该 smoke 证明模块公开 include 与最终链接依赖可传递，不证明窗口、设备创建或 Vulkan runtime 行为。

## 全新构建检查

在已经加载 MSVC 开发环境的 PowerShell 中运行：

```powershell
git status --short --branch
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake --list-presets
cmake --preset ninja-msvc
cmake --build --preset msvc-debug --target huli_vulkan --parallel 1
cmake --build --preset msvc-debug --parallel 8
cmake --build --preset msvc-debug --parallel 8
```

验收标准：

- 配置日志包含 `Configuring done` 和 `Generating done`。
- `huli_vulkan` 目标和第一次完整构建都退出 `0`。
- 未修改输入时第二次构建显示 `ninja: no work to do.`。
- `out/build/ninja-msvc/CMakeCache.txt` 中的编译器、生成器和 Vulkan SDK 与预期一致。

Clang 使用同一个开发环境，但改用独立配置和构建树：

```powershell
cmake --preset ninja-clang
cmake --build --preset clang-debug --parallel 8
```

配置日志必须识别 `Clang`，缓存中的 C/C++ 编译器必须是 `clang` / `clang++`，不能复用 `out/build/ninja-msvc`。

## Visual Studio 检查

1. 确认 Visual Studio 使用 `CMakePresets.json`，并能看到 `ninja-msvc`、`ninja-clang`、`ninja-msvc-asan`、`ninja-msvc-tracy`。
2. 修改 Vulkan SDK 环境后重启 Visual Studio。
3. 对目标 preset 执行 Delete Cache and Reconfigure。
4. 检查新生成的 `out/build/<preset>/CMakeCache.txt`，不要只看系统环境变量。

```powershell
Get-Content CMakePresets.json -Raw | ConvertFrom-Json | Out-Null
cmake --list-presets
cmake --build --list-presets
rg -n "^(CMAKE_GENERATOR|CMAKE_CONFIGURATION_TYPES|CMAKE_CXX_COMPILER|Vulkan_INCLUDE_DIR|Vulkan_LIBRARY):" `
  out/build/ninja-msvc/CMakeCache.txt
```

## 专项 preset 检查

```powershell
cmake --preset ninja-msvc-asan
cmake --build --preset msvc-asan-relwithdebinfo --parallel 8

cmake --preset ninja-msvc-tracy
cmake --build --preset msvc-tracy-release --parallel 8
```

- ASan 缓存或详细编译命令必须包含 `/fsanitize=address`，configuration 必须是 `RelWithDebInfo`。
- Tracy 缓存必须记录 `TRACY_ENABLE=ON` 和 `TRACY_CALLSTACK=ON`，configuration 必须是 `Release`。
- 以上 ASan/Tracy 快照完成于 `src/vulkan` 接入前；重新验证专项 preset 前，不能声称 `huli_vulkan` 已完成对应插桩，更不能声称运行时采集或 Vulkan 路径已经通过。

## 判断方式

- MSVC 标准头、Windows SDK 或 Vulkan SDK 找不到时，先修复开发环境或清理旧缓存。
- Clang 找不到 Windows SDK 或 MSVC 兼容库时，先确认在 Developer PowerShell 中运行，并检查 `clang` / `clang++` 是否可从 `PATH` 找到；不要把个人 LLVM 绝对路径写进共享 preset。
- 依赖下载长时间无日志时，先检查 `git`/CMake 子进程，再判断是否卡死。
- 修改 `huli_vulkan` 的 CMake 或依赖范围后，运行模块编译和临时下游链接 smoke；只有 Huli 应用目标接入后，才增加应用与 Vulkan runtime smoke。
- 外部教材或参考仓库不属于固定 smoke 入口；只有用户明确指定时才单独记录来源和验证目的。

## 文档验证

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```
