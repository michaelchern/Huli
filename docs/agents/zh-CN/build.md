# Huli Build Context

只在 CMake、构建、Visual Studio、验证命令或编译错误任务中加载本文件。

## 当前状态

- Huli 使用 CMake 3.28+、C++20、Ninja、MSVC 和 Vulkan SDK。
- 根 `CMakeLists.txt` 已声明 `project(Huli)`，会下载、配置并构建第三方依赖。
- Vulkan SDK `1.4.350.0` 是当前验证和推荐基线，不代表根 CMake 会拒绝所有其他 SDK 版本。
- 根文件末尾的 Huli 源码 `add_subdirectory(...)` 仍处于注释状态；当前构建验证依赖目标，不代表已经生成 Huli 应用、编译 `src/vulkan` 或完成 Vulkan 运行验证。

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

- `CMakeSettings.json` 提供 Ninja/MSVC 的 Debug、AddressSanitizer、Release、RelWithDebInfo 和 Tracy 配置；它是 Visual Studio 入口，不替代命令行验证。
- 首次验证优先使用 `x64-Debug`。如果该配置复用了旧 SDK 缓存，先 Delete Cache and Reconfigure。
- 修改或新增配置后，至少检查 JSON 可解析、配置目录独立，并确认实际 `CMakeCache.txt` 使用了预期编译器和 Vulkan SDK。

## 命令行配置与构建

在 Visual Studio Developer PowerShell 或已经加载 MSVC 开发环境的终端中运行：

```powershell
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake -S . -B out/build/dependency-check -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DBUILD_TESTING=OFF `
  -DGLI_TEST_ENABLE=OFF `
  -DTRACY_CALLSTACK=OFF `
  -DTRACY_ENABLE=OFF `
  -DIMGUI_EXAMPLES=OFF

cmake --build out/build/dependency-check --parallel 8
```

配置必须出现 `Configuring done` 和 `Generating done`，完整构建必须以退出代码 `0` 结束。未修改输入时再次构建应显示 `ninja: no work to do.`。

如果普通 PowerShell 找不到 MSVC 头文件或库，使用 Visual Studio Developer PowerShell，或先调用对应安装目录中的 `VsDevCmd.bat -arch=x64 -host_arch=x64`。

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
- 纯文档改动不要求重新编译，但必须运行 `tools/sync-agents.ps1 -Check` 和 `git diff --check`。
- 一次性工具版本、构建目录和验证结果写入带日期与命令的 `docs/tasks/` 文档，不要升级为无时间边界的长期规则。
- 如果只验证了第三方依赖，明确报告验证范围；不要声称 Huli 应用、`src/vulkan` 或 Vulkan 运行路径已经通过。
