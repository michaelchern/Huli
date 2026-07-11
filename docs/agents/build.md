# Huli Build Context
<!-- AGENT_DOCS_BUILD_ZH_CN_SHA256: 73d8f29993efbf0bd2f45f60599e43c2b778e2e971184e16faff45d96aefaed4 -->

Load this file only for CMake, build, Visual Studio, validation-command, or compiler-error work.

## Current State

- Huli uses CMake 3.28+, C++20, Ninja, MSVC, and the Vulkan SDK.
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

- `CMakeSettings.json` provides Ninja/MSVC Debug, AddressSanitizer, Release, RelWithDebInfo, and Tracy configurations. It is a Visual Studio entrypoint, not a replacement for command-line validation.
- Start validation with `x64-Debug`. If it reuses an old SDK cache, use Delete Cache and Reconfigure first.
- After changing or adding a configuration, at minimum validate its JSON, confirm build directories are isolated, and inspect the resulting `CMakeCache.txt` for the intended compiler and Vulkan SDK.

## Command-Line Configure and Build

Run from Visual Studio Developer PowerShell or another shell with the MSVC environment loaded:

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

Configure must report `Configuring done` and `Generating done`; the complete build must exit with code `0`. A second unchanged build should report `ninja: no work to do.`

If plain PowerShell cannot find MSVC headers or libraries, use Visual Studio Developer PowerShell or first call the installed `VsDevCmd.bat -arch=x64 -host_arch=x64`.

## Reading Common Logs

- `Performing Test CMAKE_HAVE_LIBC_PTHREAD - Failed` is not fatal on Windows when followed by `Found Threads: TRUE`.
- `FetchContent_Populate(...) is deprecated` means an obsolete download pattern remains; the current root CMake should no longer emit it.
- glslang HLSL-front-end deprecation and glTF-SDK future CMake compatibility messages are currently warnings, not configure failures.
- The DLSS repository contains large files and submodules. Inspect active download processes before declaring a quiet first download stuck.
- Diagnose the first `CMake Error`, `fatal:`, `FAILED:`, or MSVC `error Cxxxx`, rather than reasoning backward from the final exit code.

## Change and Validation Rules

- Run `git status --short --branch` before edits and preserve user changes.
- After CMake, dependency-version, or build-option changes, run a fresh configure, complete build, and `git diff --check`.
- Docs-only changes do not require a rebuild, but must pass `tools/sync-agents.ps1 -Check` and `git diff --check`.
- Put one-off tool versions, build directories, and validation results in dated `docs/tasks/` documents rather than promoting them to timeless domain rules.
- If only dependencies were validated, report that exact scope; do not claim that a Huli application, `src/vulkan`, or Vulkan runtime path passed.
