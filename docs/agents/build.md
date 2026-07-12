# Huli Build Context
<!-- AGENT_DOCS_BUILD_ZH_CN_SHA256: fbd315998827e70713b3ee42215202884324a2a17c20f00b956905aa9db092a6 -->

Load this file only for CMake, build, Visual Studio, validation-command, or compiler-error work.

## Current State

- Huli uses CMake 3.28+, C++20, Ninja Multi-Config, MSVC, Clang, and the Vulkan SDK.
- The root `CMakeLists.txt` declares `project(Huli)` and downloads, configures, and builds third-party dependencies.
- Vulkan SDK `1.4.350.0` is the current validated and recommended baseline; this does not mean the root CMake rejects every other SDK version.
- The root build connects `src/vulkan` through `add_subdirectory(src/vulkan)`, builds the `huli_vulkan` static library, and provides the `Huli::Vulkan` alias. It still does not generate a Huli application or connect the Vulkan runtime entrypoint.
- `src/vulkan/CMakeLists.txt` uses target-scoped includes, compile features, and dependency visibility. Dependencies exposed by public headers are `PUBLIC`; implementation-only dependencies are `PRIVATE`. Read the live file for the actual target list.

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
cmake --build --preset msvc-debug --target huli_vulkan --parallel 1
cmake --build --preset msvc-debug --parallel 8
cmake --build --preset msvc-debug --parallel 8
```

Configure must report `Configuring done` and `Generating done`; both the `huli_vulkan` target and the complete build must exit with code `0`. A second unchanged build should report `ninja: no work to do.`

Configure Clang with `cmake --preset ninja-clang` and use the corresponding `clang-*` build preset. If plain PowerShell cannot find MSVC headers, the Windows SDK, or libraries, use Visual Studio Developer PowerShell or first call the installed `VsDevCmd.bat -arch=x64 -host_arch=x64`. Run Clang builds in the same environment so Windows SDK discovery stays deterministic.

ASan and Tracy use isolated build trees:

```powershell
cmake --preset ninja-msvc-asan
cmake --build --preset msvc-asan-relwithdebinfo --parallel 8

cmake --preset ninja-msvc-tracy
cmake --build --preset msvc-tracy-release --parallel 8
```

The ASan preset adds `/fsanitize=address` through `CFLAGS` / `CXXFLAGS` and uses RelWithDebInfo to avoid MSVC Debug's default `/RTC1` conflict. The Tracy preset sets `TRACY_ENABLE` and `TRACY_CALLSTACK` to `ON`. The root build contains dependencies and `huli_vulkan`, but no Huli application; these configurations do not prove application instrumentation, Tracy runtime collection, or Vulkan runtime behavior.

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
- After changing `huli_vulkan` dependency visibility, compile the static library and link a temporary downstream executable against `Huli::Vulkan`; archive creation alone cannot expose every missing final-link dependency.
- Docs-only changes do not require a rebuild, but must pass `tools/sync-agents.ps1 -Check` and `git diff --check`.
- Put one-off tool versions, build directories, and validation results in dated `docs/tasks/` documents rather than promoting them to timeless domain rules.
- If only dependencies or `huli_vulkan` compilation were validated, report that exact scope; do not claim that a Huli application or Vulkan runtime path passed.
