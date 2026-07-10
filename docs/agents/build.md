# Huli Build Context
<!-- AGENT_DOCS_BUILD_ZH_CN_SHA256: d0acc13569f0f9ffe12a4f77430fde406700cbd552a449be2c81d25189c29043 -->

Load this file only for CMake, build, Visual Studio, validation-command, or compiler-error work.

## Current State

- Huli is currently a learning repository skeleton.
- The root `CMakeLists.txt` currently declares only the minimum CMake version; it has no `project()` or buildable target.
- No source, shader, or runtime entrypoint directory is currently part of the repository contract. Update this file after adding them.

## Rules

- Run `git status --short --branch` before edits.
- First decide whether a failure comes from the Huli build skeleton, MSVC/SDK environment, or Vulkan SDK.
- If Huli has no configured target yet, say the repository does not currently define a complete build target. Do not describe that as a Vulkan logic failure.
- After C++ / CMake / shader changes, run the smallest relevant command. Docs-only changes do not need a CMake build.

## Reference Commands

```powershell
git status --short --branch
if (Test-Path CMakePresets.json) { cmake --list-presets }
cmake -S . -B build
cmake --build build
```

If the MSVC environment is missing, prefer a Visual Studio Developer Command Prompt or explicitly load `VsDevCmd.bat` before retrying.
