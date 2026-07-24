# Huli Build Smoke

## 当前事实

- 根 `CMakeLists.txt` 可以配置并构建第三方依赖、`src/vulkan` 的 `huli_vulkan`、`src/render` 的 `huli_render`，以及非 Android 桌面平台的 `huli_example1`；Windows 与 macOS 有已验证 preset。
- `huli_example1` 编译链接成功不等于 Vulkan runtime smoke 通过；必须在 Vulkan validation layer 已启用时单独检查应用启动、验证层输出和持续运行状态。禁用验证层的运行不能证明 Vulkan 使用正确。
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

## 2026-07-14 应用编译与 Runtime 快照

- `cmake --preset ninja-msvc` 配置成功；`huli_vulkan`、`huli_example1` 和完整 Debug 构建均退出 `0`，未修改输入时再次构建返回 `ninja: no work to do.`。
- `huli_example1` 的 MSVC 编译命令包含 `/utf-8` 与 `-DNOMINMAX`，`C4819` 和 `std::min` 处的 `C2589` 不再出现。
- `Common.hpp` 恢复为无外层 `NOMINMAX` 防重复检查的写法后，应用仍能编译链接，但会报告 `C4005: NOMINMAX` 宏重定义；这不是 runtime 退出原因，也不应被当作干净构建。
- 在 `validationLayers` 包含 `VK_LAYER_KHRONOS_validation` 的状态下启动 Debug `huli_example1.exe`，程序完成 Vulkan instance、物理设备、surface 与逻辑设备创建路径的前置输出，但验证层首先报告 `VUID-VkDeviceCreateInfo-enabledLayerCount-12384`：当前 `VkDeviceCreateInfo` 仍把 instance validation layers 传给 device layers。
- `debugMessengerCallback` 收到 error severity 后调用 `__debugbreak()`；5 秒存活检查确认进程以 `0x80000003` 退出。因此本快照只证明应用可编译、链接并进入 Vulkan 初始化，不证明可持续运行或 runtime 验证通过。

## 2026-07-17 macOS、VS Code 与 Runtime 快照

- 验证环境为 Apple Silicon macOS 26.5.2 / Apple M5、CMake 4.3.3、Ninja 1.13.2、Apple Clang 21.0.0 和 Vulkan SDK 1.4.350.0；`out/build/ninja-macos/CMakeCache.txt` 记录 Ninja Multi-Config、`/usr/bin/c++`、`/usr/local/include` 与 `/usr/local/lib/libvulkan.dylib`。
- `CMakePresets.json` 新增 `ninja-macos` 与 `macos-debug` / `macos-release` / `macos-relwithdebinfo`，VS Code 通过 CMake Tools、clangd 与 CodeLLDB 使用同一 preset 和 launch target。
- `Context` 改为通过平台无关的 `SurfaceFactory` 创建 surface；`huli_example1` 使用 GLFW 报告的 instance extensions 和 `glfwCreateWindowSurface()`。macOS 额外启用 portability enumeration 和 portability subset，不再依赖 Win32 native window API。
- glslang 源码编译路径已从 Windows-only 改为桌面通用；Debug/Release 使用标准 `NDEBUG` 条件。简单三角形示例不再请求未使用的 descriptor-indexing 默认功能。
- 两条 `VkDeviceCreateInfo` 路径不再传递 device layers，2026-07-14 首先出现的 `VUID-VkDeviceCreateInfo-enabledLayerCount-12384` 已消除。Debug messenger 也不再声明未启用的 device-address-binding message type，默认 core validation 不再强制启用 GPU-assisted validation。
- 交换链使用实际 framebuffer/current extent，在 Retina 屏幕创建 1600×1200、3 图像 swapchain；acquire semaphore 按 frame slot 分配，render-finished semaphore 按 swapchain image 分配，避免跨帧复用仍有 pending operation 的二进制信号量。
- `cmake --preset ninja-macos` 配置成功；Debug `huli_vulkan`、Debug/Release `huli_example1` 与 Debug 完整构建均退出 `0`，未修改输入时第二次 Debug 完整构建返回 `ninja: no work to do.`。
- 在 `VK_LAYER_KHRONOS_validation` 启用时启动 Debug 应用，MoltenVK 选择 Apple M5、启用 `VK_KHR_portability_subset` 与 `VK_KHR_swapchain`，窗口持续运行约 10 秒并稳定输出约 60 FPS；期间没有 error VUID、SIGTRAP、shader module 错误或提前退出。测试进程由验证工具主动终止，未自动验证 Escape 按键路径。
- validation layer 首次读取不存在的 shader validation cache 会输出 warning；它不是 VUID，也没有中断运行。

## 全新构建检查

在已经加载 MSVC 开发环境的 PowerShell 中运行：

```powershell
git status --short --branch
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake --list-presets
cmake --preset ninja-msvc
cmake --build --preset msvc-debug --target huli_vulkan --parallel 1
cmake --build --preset msvc-debug --target huli_example1 --parallel 1
cmake --build --preset msvc-debug --parallel 8
cmake --build --preset msvc-debug --parallel 8
```

验收标准：

- 配置日志包含 `Configuring done` 和 `Generating done`。
- `huli_vulkan`、`huli_example1` 和第一次完整构建都退出 `0`。
- 未修改输入时第二次构建显示 `ninja: no work to do.`。
- `out/build/ninja-msvc/CMakeCache.txt` 中的编译器、生成器和 Vulkan SDK 与预期一致。
- 如果需要声称 runtime 通过，必须确认 Vulkan validation layer 已启用，再另行启动 `out/build/ninja-msvc/examples/example1/Debug/huli_example1.exe`，确认没有 error VUID、断点异常或提前退出。

Clang 使用同一个开发环境，但改用独立配置和构建树：

```powershell
cmake --preset ninja-clang
cmake --build --preset clang-debug --parallel 8
```

配置日志必须识别 `Clang`，缓存中的 C/C++ 编译器必须是 `clang` / `clang++`，不能复用 `out/build/ninja-msvc`。

macOS 使用独立 preset：

```bash
git status --short --branch
cmake --list-presets
cmake --preset ninja-macos
cmake --build --preset macos-debug --target huli_vulkan --parallel 8
cmake --build --preset macos-debug --target huli_example1 --parallel 8
cmake --build --preset macos-debug --parallel 8
cmake --build --preset macos-debug --parallel 8
out/build/ninja-macos/examples/example1/Debug/huli_example1
```

macOS runtime 验收要求 Debug validation layer 已启用、MoltenVK 能枚举 Apple GPU、swapchain 使用实际 framebuffer extent，并且应用持续运行时没有 error VUID 或 SIGTRAP。

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

## macOS VS Code 检查

1. 用 VS Code 打开仓库根目录，安装仓库推荐的 CMake Tools、clangd 与 CodeLLDB。
2. 执行 `CMake: Select Configure Preset` 并选择 `ninja-macos`，然后执行 `CMake: Configure`。
3. 将 launch target 设为 `huli_example1`，使用 `macos-debug` 构建。
4. 选择 `Debug huli_example1` 并按 F5；launch 配置通过 `${command:cmake.launchTargetPath}` 使用当前 CMake 目标。
5. 确认 `.vscode/settings.json` 的 `cmake.copyCompileCommands` 把当前 preset 的编译数据库复制到根目录 `compile_commands.json`，且该文件处于 Git 忽略状态。
6. 头文件若被 clangd 误报为使用旧语言标准，确认对应真实编译命令包含 `-std=c++20`，然后执行 `clangd: Restart language server`；不要为了清除误报改写合法的 C++20 源码。

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
- 修改 `huli_vulkan` 的 CMake 或依赖范围后，运行模块编译和临时下游链接 smoke；修改 `huli_render`、示例或 Windows 编译定义后，额外构建 `huli_example1`。只有在验证层启用时显式启动应用并检查首个 VUID 与持续运行状态，才能声称 Vulkan runtime smoke 通过。
- 外部教材或参考仓库不属于固定 smoke 入口；只有用户明确指定时才单独记录来源和验证目的。

## 文档验证

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```
