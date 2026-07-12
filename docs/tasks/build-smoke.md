# Huli Build Smoke
<!-- TASK_DOCS_BUILD_SMOKE_ZH_CN_SHA256: 293d38c6c7e1bfd46b31a4dcb98e601cd03d1da903ab5d7c9b8f37a728844bd1 -->

## Current Facts

- The root `CMakeLists.txt` can configure and build third-party dependencies.
- Huli application targets and `src/vulkan` are not connected to the root build, so a successful dependency build is not application compilation or Vulkan runtime validation.
- Read live dependency versions and conditions from `CMakeLists.txt`; this file stores validation entrypoints and dated evidence only.

## 2026-07-11 Validation Snapshot

- `out/build/codex-dependency-check-20260711/CMakeCache.txt` records CMake 4.3.1, MSVC 19.51, Ninja, Debug, and Vulkan SDK `1.4.350.0`.
- `ninja -C out/build/codex-dependency-check-20260711 -n` returned `ninja: no work to do.`, so that dependency build directory had no pending work at snapshot time.

## 2026-07-12 Presets Validation Snapshot

- `cmake --list-presets` lists four configure presets, and `cmake --build --list-presets` lists eight build presets.
- Fresh `ninja-msvc` and `ninja-clang` configure runs succeeded. Their caches record MSVC 19.51 and the Clang 22.1.1 GNU-style driver respectively, Ninja Multi-Config, and Vulkan SDK `1.4.350.0`.
- `msvc-debug` and `clang-debug` both completed the current root dependency build with exit code `0`. An unchanged second MSVC build returned `ninja: no work to do.`.
- `msvc-release`, `msvc-relwithdebinfo`, `clang-release`, and `clang-relwithdebinfo` each select the correct configuration ninja file within the existing toolchain build tree.
- `msvc-asan-relwithdebinfo` completed a full build. Its cache and verbose compiler commands contain `/fsanitize=address` without Debug's `/RTC1`.
- `msvc-tracy-release` completed a full build, and its cache records `TRACY_ENABLE=ON` and `TRACY_CALLSTACK=ON`.
- The root project does not currently consume the preset's `BUILD_TESTING`, `GLI_TEST_ENABLE`, or `IMGUI_EXAMPLES` values, so configure reports a manually-specified variables unused warning. This warning is not a build failure, but do not claim those switches changed behavior until the corresponding root options are connected.

## Fresh Dependency Check

Run from PowerShell with the MSVC development environment loaded:

```powershell
git status --short --branch
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake --list-presets
cmake --preset ninja-msvc
cmake --build --preset msvc-debug --parallel 8
cmake --build --preset msvc-debug --parallel 8
```

Acceptance criteria:

- Configure reports `Configuring done` and `Generating done`.
- The first complete build exits with code `0`.
- The second unchanged build reports `ninja: no work to do.`.
- `out/build/ninja-msvc/CMakeCache.txt` names the intended compiler, generator, and Vulkan SDK.

Use the same development environment for Clang, but keep its configure and build tree separate:

```powershell
cmake --preset ninja-clang
cmake --build --preset clang-debug --parallel 8
```

Configure must identify Clang, and the cached C/C++ compilers must be `clang` / `clang++`. Do not reuse `out/build/ninja-msvc`.

## Visual Studio Check

1. Confirm that Visual Studio uses `CMakePresets.json` and shows `ninja-msvc`, `ninja-clang`, `ninja-msvc-asan`, and `ninja-msvc-tracy`.
2. Restart Visual Studio after changing the Vulkan SDK environment.
3. Run Delete Cache and Reconfigure for the target preset.
4. Inspect the new `out/build/<preset>/CMakeCache.txt`; do not rely on the system environment variable alone.

```powershell
Get-Content CMakePresets.json -Raw | ConvertFrom-Json | Out-Null
cmake --list-presets
cmake --build --list-presets
rg -n "^(CMAKE_GENERATOR|CMAKE_CONFIGURATION_TYPES|CMAKE_CXX_COMPILER|Vulkan_INCLUDE_DIR|Vulkan_LIBRARY):" `
  out/build/ninja-msvc/CMakeCache.txt
```

## Specialized Preset Check

```powershell
cmake --preset ninja-msvc-asan
cmake --build --preset msvc-asan-relwithdebinfo --parallel 8

cmake --preset ninja-msvc-tracy
cmake --build --preset msvc-tracy-release --parallel 8
```

- The ASan cache or verbose compiler command must contain `/fsanitize=address`, and the configuration must be `RelWithDebInfo`.
- The Tracy cache must record `TRACY_ENABLE=ON` and `TRACY_CALLSTACK=ON`, and the configuration must be `Release`.
- The root build does not connect a Huli application or `src/vulkan`. A successful specialized preset only proves that the current dependency build accepts the configuration, not that runtime instrumentation, profiling, or the Vulkan path has passed.

## Diagnosis

- If MSVC standard headers, the Windows SDK, or the Vulkan SDK are missing, repair the development environment or clear the old cache first.
- If Clang cannot find the Windows SDK or MSVC-compatible libraries, first confirm that it runs from Developer PowerShell and that `clang` / `clang++` are available on `PATH`. Do not put a personal absolute LLVM path in the shared preset.
- If dependency download output stays quiet, inspect active `git` and CMake child processes before declaring it stuck.
- Add application build and runtime smoke only after a Huli application target is connected; until then, do not report Vulkan runtime validation.
- External study material or reference repositories are not fixed smoke entrypoints. Record their source and validation purpose only when the user explicitly selects them.

## Documentation Validation

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```
