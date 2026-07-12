# Huli Build Context

只在 CMake、构建、Visual Studio、验证命令或编译错误任务中加载本文件。

## 当前状态

- Huli 使用 CMake 3.28+、C++20、Ninja Multi-Config、MSVC、Clang 和 Vulkan SDK。
- 根 `CMakeLists.txt` 已声明 `project(Huli)`，会下载、配置并构建第三方依赖。
- Vulkan SDK `1.4.350.0` 是当前验证和推荐基线，不代表根 CMake 会拒绝所有其他 SDK 版本。
- 根文件已通过 `add_subdirectory(src/vulkan)` 接入 `huli_vulkan` 静态库，并提供 `Huli::Vulkan` 别名；尚未生成 Huli 应用或接入 Vulkan 运行入口。
- `src/vulkan/CMakeLists.txt` 使用目标级 include、编译特性和依赖范围；公开头文件暴露的依赖使用 `PUBLIC`，仅实现使用的依赖使用 `PRIVATE`，实际目标清单以实时文件为准。

## 实时权威与环境

- 当前依赖版本、条件和目标以实时 `CMakeLists.txt` 为权威，不在本上下文中维护第二份完整版本表。
- 推荐环境变量：

```text
VULKAN_SDK=C:\VulkanSDK\1.4.350.0
```

- 全新配置时应确认 `Found Vulkan` 和 `Vulkan_INCLUDE_DIR` 都指向期望 SDK。
- CMake 会复用 `CMakeCache.txt`。修改 `VULKAN_SDK` 后，如果日志仍指向旧 SDK，删除对应构建目录的缓存并重新配置。
- Visual Studio 在启动时读取环境变量。修改 SDK 环境后，应重启 Visual Studio，并执行“删除缓存并重新配置”。

## 第三方依赖策略

- 正式发布的依赖固定到明确 tag，并在适用时使用 `GIT_SHALLOW TRUE`。
- 没有正式 tag 的依赖固定到不可变提交，不跟随 `master` 或 `main`。
- `Vulkan_VERSION VERSION_GREATER_EQUAL "1.4.350"` 用于选择 glslang 获取路径，不是全局 Vulkan SDK 精确版本校验。SPIRV-Reflect、volk 等实际固定值以根 CMake 为准。
- 使用 `FetchContent_MakeAvailable(...)`，不要重新引入 CMake 4.x 已弃用的直接 `FetchContent_Populate(...)` 调用。
- RapidJSON 使用 glTF-SDK 1.9.6 所需的兼容提交；官方 `v1.1.0` 缺少该源码需要的 `SchemaDocument` 接口。
- 更新任何依赖版本后，必须重新执行全新配置和完整依赖构建，不能只确认 tag 存在。

## Visual Studio 配置

- `CMakePresets.json` 是 Visual Studio、命令行和未来 CI 共用的唯一项目配置入口；不要重新添加并行维护的 `CMakeSettings.json`。
- Windows 核心 configure presets 是 `ninja-msvc` 和 `ninja-clang`，都使用 Ninja Multi-Config；专项 presets 是 `ninja-msvc-asan` 和 `ninja-msvc-tracy`。
- MSVC 和 Clang 各提供 Debug、Release、RelWithDebInfo build preset；ASan 使用 MSVC RelWithDebInfo，Tracy 使用 MSVC Release。
- `ninja-clang` 使用 `clang` / `clang++` GNU 风格驱动，不是 `clang-cl`。编译器只按命令名查找，不能把个人 LLVM 安装绝对路径写入项目 preset。
- 本机路径、个人环境覆盖或实验配置写入已忽略的 `CMakeUserPresets.json`；`VULKAN_SDK` 继续由启动 Visual Studio 或终端的环境提供。
- 修改或新增 preset 后，至少运行 `cmake --list-presets`、确认构建目录相互隔离，并检查实际 `CMakeCache.txt` 使用了预期生成器、编译器和 Vulkan SDK。

## 命令行配置与构建

在 Visual Studio Developer PowerShell 或已经加载 MSVC 开发环境的终端中运行：

```powershell
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake --preset ninja-msvc
cmake --build --preset msvc-debug --target huli_vulkan --parallel 1
cmake --build --preset msvc-debug --parallel 8
cmake --build --preset msvc-debug --parallel 8
```

配置必须出现 `Configuring done` 和 `Generating done`，`huli_vulkan` 目标与完整构建都必须以退出代码 `0` 结束。未修改输入时再次构建应显示 `ninja: no work to do.`。

Clang 配置使用 `cmake --preset ninja-clang` 和对应的 `clang-*` build preset。普通 PowerShell 找不到 MSVC 头文件、Windows SDK 或库时，使用 Visual Studio Developer PowerShell，或先调用对应安装目录中的 `VsDevCmd.bat -arch=x64 -host_arch=x64`；Clang 构建也应在该环境中运行，以稳定获得 Windows SDK。

ASan 与 Tracy 使用独立构建树：

```powershell
cmake --preset ninja-msvc-asan
cmake --build --preset msvc-asan-relwithdebinfo --parallel 8

cmake --preset ninja-msvc-tracy
cmake --build --preset msvc-tracy-release --parallel 8
```

ASan preset 通过 `CFLAGS` / `CXXFLAGS` 添加 `/fsanitize=address`，并使用 RelWithDebInfo 避免 MSVC Debug 默认 `/RTC1` 冲突。Tracy preset 将 `TRACY_ENABLE` 和 `TRACY_CALLSTACK` 设为 `ON`。当前根构建包含依赖和 `huli_vulkan`，但没有 Huli 应用；这些专项配置不能证明应用插桩、Tracy 运行采集或 Vulkan runtime 已通过。

## 常见日志判断

- `Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed` 在 Windows 上不是最终错误；后续出现 `Found Threads: TRUE` 即表示探测成功。
- `FetchContent_Populate(...) is deprecated` 表示仍有旧下载写法；当前根 CMake 不应再产生该警告。
- glslang 的 HLSL 前端弃用提示和 glTF-SDK 的 CMake 未来兼容提示目前是 warning，不是配置失败。
- DLSS 仓库包含大文件和子模块，首次下载可能长时间无新日志；先检查下载进程再判断是否卡死。
- 排错时先找第一条 `CMake Error`、`fatal:`、`FAILED:` 或 MSVC `error Cxxxx`，不要从最后的退出代码反推原因。

## 修改与验证规则

- 修改前运行 `git status --short --branch`，不要覆盖用户已有改动。
- `.gitignore` 按生成目录和工具本地状态组织；不要全局忽略源码、shader 或 `.exe` / `.dll` / `.lib` 等可能合法提交的预编译文件。`CMakePresets.json` 可以进入版本库，`CMakeUserPresets.json` 必须保持忽略。
- 修改 `.gitignore` 后，用 `git check-ignore -v --no-index` 同时验证应忽略与不应忽略的代表路径，不要用清理命令代替规则检查。
- CMake、依赖版本或构建选项改动后，运行全新目录配置、完整构建和 `git diff --check`。
- 修改 `huli_vulkan` 的依赖范围后，除编译静态库外，还要用临时下游 executable 仅链接 `Huli::Vulkan`；静态库归档成功不能发现所有缺失的最终链接依赖。
- 纯文档改动不要求重新编译，但必须运行 `tools/sync-agents.ps1 -Check` 和 `git diff --check`。
- 一次性工具版本、构建目录和验证结果写入带日期与命令的 `docs/tasks/` 文档，不要升级为无时间边界的长期规则。
- 如果只验证了第三方依赖或 `huli_vulkan` 编译，明确报告验证范围；不要据此声称 Huli 应用或 Vulkan 运行路径已经通过。
