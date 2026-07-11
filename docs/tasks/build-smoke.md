# Huli Build Smoke
<!-- TASK_DOCS_BUILD_SMOKE_ZH_CN_SHA256: 8628f28411598cf0401b3200217cb535559f17d3eee8227da13008983c29a6f0 -->

## Current Facts

- Huli root `CMakeLists.txt` is currently a learning skeleton.
- The Cookbook repository has its own CMake project, but it is read-only reference by default.
- A Huli build failure may only mean targets are not configured yet. It is not automatically a Vulkan code failure.

## Huli Minimal Checks

```powershell
git status --short --branch
if (Test-Path CMakePresets.json) { cmake --list-presets }
cmake -S . -B build
cmake --build build
```

## Interpretation

- If CMake reports no buildable target, first add or record the missing Huli build skeleton.
- If MSVC standard headers, Windows SDK, or Vulkan SDK are missing, fix the environment first.
- If shader or Vulkan validation reports errors, then enter Vulkan-specific debugging.

## Cookbook Read-Only Validation

By default, only check the Cookbook worktree. Do not start its heavier dependency download and build:

```powershell
$cookbookRoot = 'C:\Users\12168\Documents\GitHub\The-Modern-Vulkan-Cookbook'
git -C $cookbookRoot status --short --branch
```

Only when the Cookbook build must be validated explicitly, use a build directory outside its checkout:

```powershell
$cookbookBuild = Join-Path $env:TEMP 'huli-modern-vulkan-cookbook-build'
cmake -S $cookbookRoot -B $cookbookBuild
cmake --build $cookbookBuild
```

These commands may download dependencies and take substantial time. Do not write Huli notes or experiment state into the Cookbook source tree.

## Docs Validation

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```
