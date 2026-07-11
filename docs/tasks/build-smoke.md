# Huli Build Smoke
<!-- TASK_DOCS_BUILD_SMOKE_ZH_CN_SHA256: a50324cd4fda66d143ee1e5dab15963619f402d0f7aee335d4e6a22771155408 -->

## Current Facts

- The root `CMakeLists.txt` can configure and build third-party dependencies.
- Huli application targets and `src/vulkan` are not connected to the root build, so a successful dependency build is not application compilation or Vulkan runtime validation.
- Read live dependency versions and conditions from `CMakeLists.txt`; this file stores validation entrypoints and dated evidence only.

## 2026-07-11 Validation Snapshot

- `out/build/codex-dependency-check-20260711/CMakeCache.txt` records CMake 4.3.1, MSVC 19.51, Ninja, Debug, and Vulkan SDK `1.4.350.0`.
- `ninja -C out/build/codex-dependency-check-20260711 -n` returned `ninja: no work to do.`, so that dependency build directory had no pending work at snapshot time.
- `out/build/x64-Debug/CMakeCache.txt` still records old Vulkan SDK `1.4.341.1`. Before using Visual Studio `x64-Debug`, run Delete Cache and Reconfigure instead of treating that cache as current environment evidence.

## Fresh Dependency Check

Run from PowerShell with the MSVC development environment loaded:

```powershell
git status --short --branch
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake -S . -B out/build/dependency-check -G Ninja `
  -DCMAKE_BUILD_TYPE=Debug `
  -DBUILD_TESTING=OFF `
  -DGLI_TEST_ENABLE=OFF `
  -DTRACY_CALLSTACK=OFF `
  -DTRACY_ENABLE=OFF `
  -DIMGUI_EXAMPLES=OFF

cmake --build out/build/dependency-check --parallel 8
cmake --build out/build/dependency-check --parallel 8
```

Acceptance criteria:

- Configure reports `Configuring done` and `Generating done`.
- The first complete build exits with code `0`.
- The second unchanged build reports `ninja: no work to do.`.
- `CMakeCache.txt` names the intended compiler, generator, and Vulkan SDK.

## Visual Studio Check

1. Confirm that `CMakeSettings.json` parses.
2. Restart Visual Studio after changing the Vulkan SDK environment.
3. Run Delete Cache and Reconfigure for the target configuration.
4. Inspect the new `out/build/<configuration>/CMakeCache.txt`; do not rely on the system environment variable alone.

```powershell
Get-Content CMakeSettings.json -Raw | ConvertFrom-Json | Out-Null
rg -n "^(CMAKE_GENERATOR|CMAKE_BUILD_TYPE|CMAKE_CXX_COMPILER|Vulkan_INCLUDE_DIR|Vulkan_LIBRARY):" `
  out/build/x64-Debug/CMakeCache.txt
```

## Diagnosis

- If MSVC standard headers, the Windows SDK, or the Vulkan SDK are missing, repair the development environment or clear the old cache first.
- If dependency download output stays quiet, inspect active `git` and CMake child processes before declaring it stuck.
- Add application build and runtime smoke only after a Huli application target is connected; until then, do not report Vulkan runtime validation.
- External study material or reference repositories are not fixed smoke entrypoints. Record their source and validation purpose only when the user explicitly selects them.

## Documentation Validation

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```
