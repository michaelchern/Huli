# Huli Build Context

只在 CMake、构建、Visual Studio、验证命令或编译错误任务中加载本文件。

## 当前状态

- Huli 使用 CMake 3.28+、C++20、Ninja Multi-Config、MSVC、Clang、Apple Clang 和 Vulkan SDK。
- 根 `CMakeLists.txt` 已声明 `project(Huli)`，会下载、配置并构建第三方依赖。
- Vulkan SDK `1.4.350.0` 是当前验证和推荐基线，不代表根 CMake 会拒绝所有其他 SDK 版本。
- 根文件已接入 `huli_vulkan` 静态库、`huli_render` 头文件目标，并在非 Android 桌面平台生成 `huli_example1`。Windows 与 macOS 有独立 preset；目标编译链接成功不代表 Vulkan runtime smoke 已通过。
- `src/vulkan/CMakeLists.txt` 使用目标级 include、编译特性和依赖范围；公开头文件暴露的依赖使用 `PUBLIC`，仅实现使用的依赖使用 `PRIVATE`，实际目标清单以实时文件为准。

## 实时权威与环境

- 当前依赖版本、条件和目标以实时 `CMakeLists.txt` 为权威，不在本上下文中维护第二份完整版本表。
- Windows 推荐环境变量：

```text
VULKAN_SDK=C:\VulkanSDK\1.4.350.0
```

- 全新配置时应确认 `Found Vulkan` 和 `Vulkan_INCLUDE_DIR` 都指向期望 SDK。
- CMake 会复用 `CMakeCache.txt`。修改 `VULKAN_SDK` 后，如果日志仍指向旧 SDK，删除对应构建目录的缓存并重新配置。
- Visual Studio 在启动时读取环境变量。修改 SDK 环境后，应重启 Visual Studio，并执行“删除缓存并重新配置”。
- macOS 可使用 LunarG Vulkan SDK / MoltenVK 的系统安装；共享 preset 不写个人 SDK 绝对路径。全新配置必须确认 Vulkan include、loader 和运行时 ICD 来自同一套期望安装。

## 第三方依赖策略

- 正式发布的依赖固定到明确 tag，并在适用时使用 `GIT_SHALLOW TRUE`。
- 没有正式 tag 的依赖固定到不可变提交，不跟随 `master` 或 `main`。
- `Vulkan_VERSION VERSION_GREATER_EQUAL "1.4.350"` 用于选择 glslang 获取路径，不是全局 Vulkan SDK 精确版本校验。SPIRV-Reflect、volk 等实际固定值以根 CMake 为准。
- 使用 `FetchContent_MakeAvailable(...)`，不要重新引入 CMake 4.x 已弃用的直接 `FetchContent_Populate(...)` 调用。
- RapidJSON 使用 glTF-SDK 1.9.6 所需的兼容提交；官方 `v1.1.0` 缺少该源码需要的 `SchemaDocument` 接口。
- 更新任何依赖版本后，必须重新执行全新配置和完整依赖构建，不能只确认 tag 存在。

## IDE 与 preset 配置

- `CMakePresets.json` 是 Visual Studio、命令行和未来 CI 共用的唯一项目配置入口；不要重新添加并行维护的 `CMakeSettings.json`。
- Windows 核心 configure presets 是 `ninja-msvc` 和 `ninja-clang`，都使用 Ninja Multi-Config；专项 presets 是 `ninja-msvc-asan` 和 `ninja-msvc-tracy`。
- MSVC 和 Clang 各提供 Debug、Release、RelWithDebInfo build preset；ASan 使用 MSVC RelWithDebInfo，Tracy 使用 MSVC Release。
- macOS configure preset 是 `ninja-macos`，使用 Ninja Multi-Config 与系统 Apple Clang；build presets 是 `macos-debug`、`macos-release` 和 `macos-relwithdebinfo`。
- `ninja-clang` 使用 `clang` / `clang++` GNU 风格驱动，不是 `clang-cl`。编译器只按命令名查找，不能把个人 LLVM 安装绝对路径写入项目 preset。
- Huli 自有源码和公开头文件包含 UTF-8 中文注释；MSVC 目标必须通过目标级编译选项使用 `/utf-8`，头文件目标还要用 `PUBLIC` 或 `INTERFACE` 把该要求传给消费者。遇到 `C4819` 时不要删除中文注释或逐文件添加 BOM 来掩盖配置缺失。
- Windows 目标若可能在 `windows.h` 之后解析 `std::min` / `std::max`，应通过目标级 compile definition 在预处理开始前提供 `NOMINMAX`；不要依赖较晚的头文件定义或 include 顺序。头文件若保留本地兜底定义，必须先检查 `NOMINMAX` 是否已经存在，避免 `C4005`。
- 本机路径、个人环境覆盖或实验配置写入已忽略的 `CMakeUserPresets.json`；`VULKAN_SDK` 继续由启动 Visual Studio 或终端的环境提供。
- VS Code 必须打开仓库根目录并使用 CMake Tools 选择 preset 和 `huli_example1` launch target。仓库内 `.vscode/launch.json` 通过 CodeLLDB 启动 `${command:cmake.launchTargetPath}`，不会固定可执行文件绝对路径。
- 修改或新增 preset 后，至少运行 `cmake --list-presets`、确认构建目录相互隔离，并检查实际 `CMakeCache.txt` 使用了预期生成器、编译器和 Vulkan SDK。

## 命令行配置与构建

在 Visual Studio Developer PowerShell 或已经加载 MSVC 开发环境的终端中运行：

```powershell
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake --preset ninja-msvc
cmake --build --preset msvc-debug --target huli_vulkan --parallel 1
cmake --build --preset msvc-debug --target huli_example1 --parallel 1
cmake --build --preset msvc-debug --parallel 8
cmake --build --preset msvc-debug --parallel 8
```

配置必须出现 `Configuring done` 和 `Generating done`，`huli_vulkan`、`huli_example1` 与完整构建都必须以退出代码 `0` 结束。未修改输入时再次构建应显示 `ninja: no work to do.`；这些结果只证明编译链接，不替代应用启动与 Vulkan 验证层检查。

Clang 配置使用 `cmake --preset ninja-clang` 和对应的 `clang-*` build preset。普通 PowerShell 找不到 MSVC 头文件、Windows SDK 或库时，使用 Visual Studio Developer PowerShell，或先调用对应安装目录中的 `VsDevCmd.bat -arch=x64 -host_arch=x64`；Clang 构建也应在该环境中运行，以稳定获得 Windows SDK。

macOS 使用独立构建树：

```bash
cmake --preset ninja-macos
cmake --build --preset macos-debug --target huli_vulkan --parallel 8
cmake --build --preset macos-debug --target huli_example1 --parallel 8
cmake --build --preset macos-debug --parallel 8
cmake --build --preset macos-debug --parallel 8
```

配置缓存必须记录 Ninja Multi-Config、Apple Clang 和期望的 Vulkan SDK。Debug `huli_example1` 使用标准 `NDEBUG` 条件启用 validation layer；运行时必须检查首个 VUID 和持续进程状态。macOS 通过 GLFW 创建 Metal surface，并为 MoltenVK 启用 portability enumeration/subset。

ASan 与 Tracy 使用独立构建树：

```powershell
cmake --preset ninja-msvc-asan
cmake --build --preset msvc-asan-relwithdebinfo --parallel 8

cmake --preset ninja-msvc-tracy
cmake --build --preset msvc-tracy-release --parallel 8
```

ASan preset 通过 `CFLAGS` / `CXXFLAGS` 添加 `/fsanitize=address`，并使用 RelWithDebInfo 避免 MSVC Debug 默认 `/RTC1` 冲突。Tracy preset 将 `TRACY_ENABLE` 和 `TRACY_CALLSTACK` 设为 `ON`。当前根构建虽包含 `huli_example1`，但旧的专项 preset 快照早于该应用接入；重新验证前不能声称应用插桩、Tracy 运行采集或 Vulkan runtime 已通过。

## 常见日志判断

- `Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed` 在 Windows 上不是最终错误；后续出现 `Found Threads: TRUE` 即表示探测成功。
- `FetchContent_Populate(...) is deprecated` 表示仍有旧下载写法；当前根 CMake 不应再产生该警告。
- glslang 的 HLSL 前端弃用提示和 glTF-SDK 的 CMake 未来兼容提示目前是 warning，不是配置失败。
- validation layer 首次运行时无法读取尚不存在的 shader validation cache 属于 warning；不能与 VUID 混为一谈。
- `VK_SUBOPTIMAL_KHR` 表示当前 acquire/present 仍可完成，但交换链已非最佳状态；不能统一交给只接受 `VK_SUCCESS` 的错误宏。
- DLSS 仓库包含大文件和子模块，首次下载可能长时间无新日志；先检查下载进程再判断是否卡死。
- MSVC `C4819` 后紧跟类声明、预处理指令等连带语法错误时，先检查真实编译命令是否包含 `/utf-8`；不要从后续错误行反推代码损坏。
- `std::min` / `std::max` 附近的 `C2589` 常见于 Windows `min` / `max` 宏展开，检查 `NOMINMAX` 是否从编译开始生效；`C4005: NOMINMAX` 则表示命令行定义与头文件兜底重复且未加保护。
- 排错时先找第一条 `CMake Error`、`fatal:`、`FAILED:` 或 MSVC `error Cxxxx`，不要从最后的退出代码反推原因。

## 修改与验证规则

- 修改前运行 `git status --short --branch`，不要覆盖用户已有改动。
- `.gitignore` 按生成目录和工具本地状态组织；不要全局忽略源码、shader 或 `.exe` / `.dll` / `.lib` 等可能合法提交的预编译文件。`CMakePresets.json` 可以进入版本库，`CMakeUserPresets.json` 必须保持忽略。
- 修改 `.gitignore` 后，用 `git check-ignore -v --no-index` 同时验证应忽略与不应忽略的代表路径，不要用清理命令代替规则检查。
- CMake、依赖版本或构建选项改动后，运行全新目录配置、完整构建和 `git diff --check`。
- 修改 `huli_vulkan` 的依赖范围后，除编译静态库外，还要用临时下游 executable 仅链接 `Huli::Vulkan`；静态库归档成功不能发现所有缺失的最终链接依赖。
- 纯文档改动不要求重新编译，但必须运行 `tools/sync-agents.ps1 -Check` 和 `git diff --check`。
- 一次性工具版本、构建目录和验证结果写入带日期与命令的 `docs/tasks/` 文档，不要升级为无时间边界的长期规则。
- 如果只验证了第三方依赖、`huli_vulkan` 或 `huli_example1` 编译链接，明确报告验证范围；不要据此声称 Vulkan 运行路径已经通过。runtime smoke 必须在 Vulkan validation layer 已启用时单独启动应用、保留第一条 VUID，并区分验证回调断点与真正根因；禁用验证层只能证明未检查状态下的启动行为。
