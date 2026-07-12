# Huli Build Context
<!-- AGENT_DOCS_BUILD_ZH_CN_SHA256: 6da0ef8e04fe9a8c996cafdbf8bc262d00c6a4665ade1ac08252819f63c13160 -->

Load this file only for CMake, build, Visual Studio, validation-command, or compiler-error work.

## Current State

- Huli uses CMake 3.28+, C++20, Ninja Multi-Config, MSVC, Clang, and the Vulkan SDK.
- The root `CMakeLists.txt` declares `project(Huli)` and downloads, configures, and builds third-party dependencies.
- Vulkan SDK `1.4.350.0` is the current validated and recommended baseline; this does not mean the root CMake rejects every other SDK version.
- Huli source `add_subdirectory(...)` calls at the end of the root file remain commented out. The current build validates dependency targets; it does not generate a Huli application, compile `src/vulkan`, or validate a Vulkan runtime path.

## Live Authority and Environment

- The live `CMakeLists.txt` is authoritative for dependency versions, conditions, and targets. Do not maintain a second complete version table in this context.
- Recommended environment variable:

```text
VULKAN_SDK=C:\VulkanSDK\1.4.350.0
```

- On a fresh configure, confirm that both `Found Vulkan` and `Vulkan_INCLUDE_DIR` point to the intended SDK.
- CMake reuses `CMakeCache.txt`. After changing `VULKAN_SDK`, delete the matching build cache and reconfigure if logs still name the old SDK.
- Visual Studio reads environment variables at startup. Restart it after changing the SDK, then use Delete Cache and Reconfigure.

## Third-Party Dependency Policy

- Pin released dependencies to explicit tags and use `GIT_SHALLOW TRUE` where appropriate.
- Pin repositories without release tags to immutable commits, never floating `master` or `main`.
- `Vulkan_VERSION VERSION_GREATER_EQUAL "1.4.350"` selects the glslang acquisition path; it is not a global exact-SDK check. Read the root CMake for the current SPIRV-Reflect, volk, and other pins.
- Use `FetchContent_MakeAvailable(...)`; do not reintroduce direct `FetchContent_Populate(...)` calls deprecated by CMake 4.x.
- RapidJSON uses the compatibility commit required by glTF-SDK 1.9.6; official `v1.1.0` lacks the `SchemaDocument` interface that source expects.
- After changing any dependency version, run a fresh configure and a complete dependency build. Verifying that a tag exists is not sufficient.

## Visual Studio Configuration

- `CMakePresets.json` is the single project configuration entrypoint shared by Visual Studio, the command line, and future CI. Do not reintroduce a parallel `CMakeSettings.json`.
- The core Windows configure presets are `ninja-msvc` and `ninja-clang`, both using Ninja Multi-Config. The specialized presets are `ninja-msvc-asan` and `ninja-msvc-tracy`.
- MSVC and Clang each provide Debug, Release, and RelWithDebInfo build presets. ASan uses MSVC RelWithDebInfo, while Tracy uses MSVC Release.
- `ninja-clang` uses the GNU-style `clang` / `clang++` drivers, not `clang-cl`. Compilers are resolved by command name; do not put a developer's absolute LLVM path in the project preset.
- Put machine paths, personal environment overrides, and experimental configurations in the ignored `CMakeUserPresets.json`. `VULKAN_SDK` continues to come from the environment that launches Visual Studio or the terminal.
- After changing or adding a preset, run `cmake --list-presets`, confirm build directories are isolated, and inspect the resulting `CMakeCache.txt` for the intended generator, compiler, and Vulkan SDK.

## Command-Line Configure and Build

Run from Visual Studio Developer PowerShell or another shell with the MSVC environment loaded:

```powershell
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake --preset ninja-msvc
cmake --build --preset msvc-debug --parallel 8
cmake --build --preset msvc-debug --parallel 8
```

Configure must report `Configuring done` and `Generating done`; the complete build must exit with code `0`. A second unchanged build should report `ninja: no work to do.`

Configure Clang with `cmake --preset ninja-clang` and use the corresponding `clang-*` build preset. If plain PowerShell cannot find MSVC headers, the Windows SDK, or libraries, use Visual Studio Developer PowerShell or first call the installed `VsDevCmd.bat -arch=x64 -host_arch=x64`. Run Clang builds in the same environment so Windows SDK discovery stays deterministic.

ASan and Tracy use isolated build trees:

```powershell
cmake --preset ninja-msvc-asan
cmake --build --preset msvc-asan-relwithdebinfo --parallel 8

cmake --preset ninja-msvc-tracy
cmake --build --preset msvc-tracy-release --parallel 8
```

The ASan preset adds `/fsanitize=address` through `CFLAGS` / `CXXFLAGS` and uses RelWithDebInfo to avoid MSVC Debug's default `/RTC1` conflict. The Tracy preset sets `TRACY_ENABLE` and `TRACY_CALLSTACK` to `ON`. The root build still covers third-party dependencies only; these specialized configurations do not prove that a Huli application is instrumented or collecting Tracy data.

## Reading Common Logs

- `Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed` is not fatal on Windows when followed by `Found Threads: TRUE`.
- `FetchContent_Populate(...) is deprecated` means an obsolete download pattern remains; the current root CMake should no longer emit it.
- glslang HLSL-front-end deprecation and glTF-SDK future CMake compatibility messages are currently warnings, not configure failures.
- The DLSS repository contains large files and submodules. Inspect active download processes before declaring a quiet first download stuck.
- Diagnose the first `CMake Error`, `fatal:`, `FAILED:`, or MSVC `error Cxxxx`, rather than reasoning backward from the final exit code.

## Change and Validation Rules

- Run `git status --short --branch` before edits and preserve user changes.
- Organize `.gitignore` around generated directories and tool-local state. Do not globally ignore source files, shaders, or potentially intentional prebuilt files such as `.exe`, `.dll`, and `.lib`. `CMakePresets.json` may be version-controlled; `CMakeUserPresets.json` must stay ignored.
- After changing `.gitignore`, use `git check-ignore -v --no-index` to test representative paths that should and should not be ignored. Do not substitute cleanup commands for rule validation.
- After CMake, dependency-version, or build-option changes, run a fresh configure, complete build, and `git diff --check`.
- Docs-only changes do not require a rebuild, but must pass `tools/sync-agents.ps1 -Check` and `git diff --check`.
- Put one-off tool versions, build directories, and validation results in dated `docs/tasks/` documents rather than promoting them to timeless domain rules.
- If only dependencies were validated, report that exact scope; do not claim that a Huli application, `src/vulkan`, or Vulkan runtime path passed.
