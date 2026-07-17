# Huli Build Context
<!-- AGENT_DOCS_BUILD_ZH_CN_SHA256: e4792e797171f4549af829f9fad7cbbea594233b6bcf23e866fbddd4041e5a6f -->

Load this file only for CMake, build, Visual Studio, validation-command, or compiler-error work.

## Current State

- Huli uses CMake 3.28+, C++20, Ninja Multi-Config, MSVC, Clang, Apple Clang, and the Vulkan SDK.
- The root `CMakeLists.txt` declares `project(Huli)` and downloads, configures, and builds third-party dependencies.
- Vulkan SDK `1.4.350.0` is the current validated and recommended baseline; this does not mean the root CMake rejects every other SDK version.
- The root build connects the `huli_vulkan` static library and the header-only `huli_render` target, and generates `huli_example1` on non-Android desktop platforms. Windows and macOS use separate presets. Successful target compilation and linking do not prove that the Vulkan runtime smoke passes.
- `src/vulkan/CMakeLists.txt` uses target-scoped includes, compile features, and dependency visibility. Dependencies exposed by public headers are `PUBLIC`; implementation-only dependencies are `PRIVATE`. Read the live file for the actual target list.

## Live Authority and Environment

- The live `CMakeLists.txt` is authoritative for dependency versions, conditions, and targets. Do not maintain a second complete version table in this context.
- Recommended Windows environment variable:

```text
VULKAN_SDK=C:\VulkanSDK\1.4.350.0
```

- On a fresh configure, confirm that both `Found Vulkan` and `Vulkan_INCLUDE_DIR` point to the intended SDK.
- CMake reuses `CMakeCache.txt`. After changing `VULKAN_SDK`, delete the matching build cache and reconfigure if logs still name the old SDK.
- Visual Studio reads environment variables at startup. Restart it after changing the SDK, then use Delete Cache and Reconfigure.
- macOS may use a system installation of the LunarG Vulkan SDK and MoltenVK. Shared presets must not contain a developer-specific SDK path. A fresh configure must confirm that the Vulkan headers, loader, and runtime ICD come from the intended installation.

## Third-Party Dependency Policy

- Pin released dependencies to explicit tags and use `GIT_SHALLOW TRUE` where appropriate.
- Pin repositories without release tags to immutable commits, never floating `master` or `main`.
- `Vulkan_VERSION VERSION_GREATER_EQUAL "1.4.350"` selects the glslang acquisition path; it is not a global exact-SDK check. Read the root CMake for the current SPIRV-Reflect, volk, and other pins.
- Use `FetchContent_MakeAvailable(...)`; do not reintroduce direct `FetchContent_Populate(...)` calls deprecated by CMake 4.x.
- RapidJSON uses the compatibility commit required by glTF-SDK 1.9.6; official `v1.1.0` lacks the `SchemaDocument` interface that source expects.
- After changing any dependency version, run a fresh configure and a complete dependency build. Verifying that a tag exists is not sufficient.

## IDE and Preset Configuration

- `CMakePresets.json` is the single project configuration entrypoint shared by Visual Studio, the command line, and future CI. Do not reintroduce a parallel `CMakeSettings.json`.
- The core Windows configure presets are `ninja-msvc` and `ninja-clang`, both using Ninja Multi-Config. The specialized presets are `ninja-msvc-asan` and `ninja-msvc-tracy`.
- MSVC and Clang each provide Debug, Release, and RelWithDebInfo build presets. ASan uses MSVC RelWithDebInfo, while Tracy uses MSVC Release.
- The macOS configure preset is `ninja-macos`, using Ninja Multi-Config and the system Apple Clang. Its build presets are `macos-debug`, `macos-release`, and `macos-relwithdebinfo`.
- `ninja-clang` uses the GNU-style `clang` / `clang++` drivers, not `clang-cl`. Compilers are resolved by command name; do not put a developer's absolute LLVM path in the project preset.
- Huli-owned sources and public headers contain UTF-8 Chinese comments. MSVC targets must use target-scoped `/utf-8`, and header targets must propagate that requirement with `PUBLIC` or `INTERFACE`. Do not hide `C4819` by removing Chinese comments or adding per-file BOMs.
- Windows targets that may parse `std::min` or `std::max` after `windows.h` must provide `NOMINMAX` as a target-scoped compile definition before preprocessing starts. Do not rely on a later header definition or include order. If a header keeps a fallback definition, guard it against an existing `NOMINMAX` to avoid `C4005`.
- Put machine paths, personal environment overrides, and experimental configurations in the ignored `CMakeUserPresets.json`. `VULKAN_SDK` continues to come from the environment that launches Visual Studio or the terminal.
- VS Code must open the repository root and use CMake Tools to select the preset and `huli_example1` launch target. The repository `.vscode/launch.json` delegates to CodeLLDB through `${command:cmake.launchTargetPath}` instead of fixing an executable path.
- After changing or adding a preset, run `cmake --list-presets`, confirm build directories are isolated, and inspect the resulting `CMakeCache.txt` for the intended generator, compiler, and Vulkan SDK.

## Command-Line Configure and Build

Run from Visual Studio Developer PowerShell or another shell with the MSVC environment loaded:

```powershell
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake --preset ninja-msvc
cmake --build --preset msvc-debug --target huli_vulkan --parallel 1
cmake --build --preset msvc-debug --target huli_example1 --parallel 1
cmake --build --preset msvc-debug --parallel 8
cmake --build --preset msvc-debug --parallel 8
```

Configure must report `Configuring done` and `Generating done`; the `huli_vulkan` and `huli_example1` targets plus the complete build must exit with code `0`. A second unchanged build should report `ninja: no work to do.` These results prove compilation and linking only, not application startup or Vulkan validation.

Configure Clang with `cmake --preset ninja-clang` and use the corresponding `clang-*` build preset. If plain PowerShell cannot find MSVC headers, the Windows SDK, or libraries, use Visual Studio Developer PowerShell or first call the installed `VsDevCmd.bat -arch=x64 -host_arch=x64`. Run Clang builds in the same environment so Windows SDK discovery stays deterministic.

macOS uses its own build tree:

```bash
cmake --preset ninja-macos
cmake --build --preset macos-debug --target huli_vulkan --parallel 8
cmake --build --preset macos-debug --target huli_example1 --parallel 8
cmake --build --preset macos-debug --parallel 8
cmake --build --preset macos-debug --parallel 8
```

The cache must record Ninja Multi-Config, Apple Clang, and the intended Vulkan SDK. Debug `huli_example1` uses the standard `NDEBUG` condition to enable the validation layer; runtime validation must inspect the first VUID and sustained process state. On macOS, GLFW creates the Metal surface and Huli enables MoltenVK portability enumeration/subset support.

ASan and Tracy use isolated build trees:

```powershell
cmake --preset ninja-msvc-asan
cmake --build --preset msvc-asan-relwithdebinfo --parallel 8

cmake --preset ninja-msvc-tracy
cmake --build --preset msvc-tracy-release --parallel 8
```

The ASan preset adds `/fsanitize=address` through `CFLAGS` / `CXXFLAGS` and uses RelWithDebInfo to avoid MSVC Debug's default `/RTC1` conflict. The Tracy preset sets `TRACY_ENABLE` and `TRACY_CALLSTACK` to `ON`. Although the root now contains `huli_example1`, the existing specialized-preset snapshots predate that integration; rerun them before claiming application instrumentation, Tracy runtime collection, or Vulkan runtime behavior.

## Reading Common Logs

- `Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed` is not fatal on Windows when followed by `Found Threads: TRUE`.
- `FetchContent_Populate(...) is deprecated` means an obsolete download pattern remains; the current root CMake should no longer emit it.
- glslang HLSL-front-end deprecation and glTF-SDK future CMake compatibility messages are currently warnings, not configure failures.
- A first validation-layer run may warn that its shader validation cache does not exist yet; this is not a VUID.
- `VK_SUBOPTIMAL_KHR` means the current acquire/present operation can still complete even though the swapchain is no longer optimal; do not route it through a helper that accepts only `VK_SUCCESS`.
- The DLSS repository contains large files and submodules. Inspect active download processes before declaring a quiet first download stuck.
- If MSVC reports `C4819` followed by cascading syntax errors near class declarations or preprocessor directives, first verify that the real compile command contains `/utf-8`; do not infer source corruption from the later line.
- `C2589` near `std::min` or `std::max` commonly means the Windows `min` or `max` macro expanded; verify that `NOMINMAX` is active from compile start. `C4005: NOMINMAX` means a command-line definition and an unguarded header fallback overlap.
- Diagnose the first `CMake Error`, `fatal:`, `FAILED:`, or MSVC `error Cxxxx`, rather than reasoning backward from the final exit code.

## Change and Validation Rules

- Run `git status --short --branch` before edits and preserve user changes.
- Organize `.gitignore` around generated directories and tool-local state. Do not globally ignore source files, shaders, or potentially intentional prebuilt files such as `.exe`, `.dll`, and `.lib`. `CMakePresets.json` may be version-controlled; `CMakeUserPresets.json` must stay ignored.
- After changing `.gitignore`, use `git check-ignore -v --no-index` to test representative paths that should and should not be ignored. Do not substitute cleanup commands for rule validation.
- After CMake, dependency-version, or build-option changes, run a fresh configure, complete build, and `git diff --check`.
- After changing `huli_vulkan` dependency visibility, compile the static library and link a temporary downstream executable against `Huli::Vulkan`; archive creation alone cannot expose every missing final-link dependency.
- Docs-only changes do not require a rebuild, but must pass `tools/sync-agents.ps1 -Check` and `git diff --check`.
- Put one-off tool versions, build directories, and validation results in dated `docs/tasks/` documents rather than promoting them to timeless domain rules.
- If only dependencies, `huli_vulkan`, or `huli_example1` compilation/linking were validated, report that exact scope; do not claim that the Vulkan runtime passed. A runtime smoke must launch the application with the Vulkan validation layer enabled, preserve the first VUID, and distinguish a validation-callback breakpoint from the underlying cause. A run with validation disabled proves only unchecked startup behavior.
