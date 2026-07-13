# Huli Build Smoke
<!-- TASK_DOCS_BUILD_SMOKE_ZH_CN_SHA256: c73e00800066859819bcd9e285e3ab85cde8410ad2390f732e08eb846aff241d -->

## Current Facts

- The root `CMakeLists.txt` configures and builds third-party dependencies, `huli_vulkan` from `src/vulkan`, `huli_render` from `src/render`, and `huli_example1` on Windows.
- Successful `huli_example1` compilation and linking do not prove that the Vulkan runtime smoke passes. Check application startup, validation output, and sustained process state separately with the Vulkan validation layer enabled. A run with validation disabled does not prove correct Vulkan usage.
- Read live dependency versions and conditions from `CMakeLists.txt`; this file stores validation entrypoints and dated evidence only.

## 2026-07-11 Validation Snapshot

- `out/build/codex-dependency-check-20260711/CMakeCache.txt` records CMake 4.3.1, MSVC 19.51, Ninja, Debug, and Vulkan SDK `1.4.350.0`.
- `ninja -C out/build/codex-dependency-check-20260711 -n` returned `ninja: no work to do.`, so that dependency build directory had no pending work at snapshot time.

## 2026-07-12 Presets Validation Snapshot

- `cmake --list-presets` lists four configure presets, and `cmake --build --list-presets` lists eight build presets.
- Fresh `ninja-msvc` and `ninja-clang` configure runs succeeded. Their caches record MSVC 19.51 and the Clang 22.1.1 GNU-style driver respectively, Ninja Multi-Config, and Vulkan SDK `1.4.350.0`.
- Before `src/vulkan` integration, `msvc-debug` and `clang-debug` both completed the then-current root dependency build with exit code `0`. An unchanged second MSVC build returned `ninja: no work to do.`.
- `msvc-release`, `msvc-relwithdebinfo`, `clang-release`, and `clang-relwithdebinfo` each select the correct configuration ninja file within the existing toolchain build tree.
- `msvc-asan-relwithdebinfo` completed a full build. Its cache and verbose compiler commands contain `/fsanitize=address` without Debug's `/RTC1`.
- `msvc-tracy-release` completed a full build, and its cache records `TRACY_ENABLE=ON` and `TRACY_CALLSTACK=ON`.
- The root project does not currently consume the preset's `BUILD_TESTING`, `GLI_TEST_ENABLE`, or `IMGUI_EXAMPLES` values, so configure reports a manually-specified variables unused warning. This warning is not a build failure, but do not claim those switches changed behavior until the corresponding root options are connected.

## 2026-07-12 Vulkan Module Integration Snapshot

- The root CMake connects `src/vulkan` through `add_subdirectory(src/vulkan)`, builds `huli_vulkan`, and provides the `Huli::Vulkan` alias.
- `cmake --build --preset msvc-debug --target huli_vulkan --parallel 1` and the corresponding Clang Debug target build both exited `0`, proving that the `src/vulkan` sources compile.
- A temporary downstream executable linked only `Huli::Vulkan`, included `<vulkan/Context.hpp>`, and referenced both a Huli Vulkan symbol and `GetDefaultResources()`. Its MSVC compile, link, and run all exited `0`.
- This smoke proves public include and final-link dependency propagation. It does not validate window creation, device creation, or Vulkan runtime behavior.

## 2026-07-14 Application Build and Runtime Snapshot

- `cmake --preset ninja-msvc` configured successfully. `huli_vulkan`, `huli_example1`, and the complete Debug build exited `0`; an unchanged follow-up build returned `ninja: no work to do.`.
- The MSVC command for `huli_example1` contains `/utf-8` and `-DNOMINMAX`; `C4819` and the `C2589` at `std::min` no longer occur.
- After `Common.hpp` was restored without an outer duplicate guard for `NOMINMAX`, the application still compiled and linked but reported `C4005: NOMINMAX` redefinition. This is not the runtime exit cause and is not a clean build.
- Starting the Debug executable while `validationLayers` contained `VK_LAYER_KHRONOS_validation` reached Vulkan instance, physical-device, surface, and logical-device initialization output, then validation first reported `VUID-VkDeviceCreateInfo-enabledLayerCount-12384`: the current `VkDeviceCreateInfo` still passes instance validation layers as device layers.
- `debugMessengerCallback` calls `__debugbreak()` for error severity. A five-second liveness check confirmed exit status `0x80000003`. This snapshot proves that the application compiles, links, and enters Vulkan initialization; it does not prove sustained execution or a passing runtime smoke.

## Fresh Build Check

Run from PowerShell with the MSVC development environment loaded:

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

Acceptance criteria:

- Configure reports `Configuring done` and `Generating done`.
- `huli_vulkan`, `huli_example1`, and the first complete build exit with code `0`.
- The second unchanged build reports `ninja: no work to do.`.
- `out/build/ninja-msvc/CMakeCache.txt` names the intended compiler, generator, and Vulkan SDK.
- To claim a passing runtime, first confirm that the Vulkan validation layer is enabled, then start `out/build/ninja-msvc/examples/example1/Debug/huli_example1.exe` and confirm that no error VUID, breakpoint exception, or early exit occurs.

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
- The ASan and Tracy snapshots above predate `src/vulkan` integration. Until those specialized presets are rerun, do not claim that `huli_vulkan` is instrumented, and never treat them as runtime collection or Vulkan-path validation.

## Diagnosis

- If MSVC standard headers, the Windows SDK, or the Vulkan SDK are missing, repair the development environment or clear the old cache first.
- If Clang cannot find the Windows SDK or MSVC-compatible libraries, first confirm that it runs from Developer PowerShell and that `clang` / `clang++` are available on `PATH`. Do not put a personal absolute LLVM path in the shared preset.
- If dependency download output stays quiet, inspect active `git` and CMake child processes before declaring it stuck.
- After changing `huli_vulkan` CMake or dependency visibility, run the module build and a temporary downstream link smoke. After changing `huli_render`, an example, or Windows compile definitions, also build `huli_example1`. Claim a passing Vulkan runtime smoke only after explicitly starting the application with validation enabled and checking the first VUID plus sustained process state.
- External study material or reference repositories are not fixed smoke entrypoints. Record their source and validation purpose only when the user explicitly selects them.

## Documentation Validation

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```
