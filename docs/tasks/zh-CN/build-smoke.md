# Huli Build Smoke

## 当前事实

- Huli 根 `CMakeLists.txt` 目前是学习骨架。
- Cookbook 仓库有自己的 CMake 工程，但默认只读参考。
- Huli 构建失败可能只是因为目标尚未配置，不一定是 Vulkan 代码失败。

## Huli 最小检查

```powershell
git status --short --branch
if (Test-Path CMakePresets.json) { cmake --list-presets }
cmake -S . -B build
cmake --build build
```

## 判断方式

- 如果 CMake 报没有可构建目标，先补 Huli 构建骨架或记录为未配置状态。
- 如果 MSVC 标准头、Windows SDK 或 Vulkan SDK 找不到，先修环境。
- 如果 shader 或 Vulkan validation 报错，再进入 Vulkan 具体排查。

## Cookbook 只读验证入口

默认只检查 Cookbook 工作树，不运行其较重的依赖下载和构建：

```powershell
$cookbookRoot = 'C:\Users\12168\Documents\GitHub\The-Modern-Vulkan-Cookbook'
git -C $cookbookRoot status --short --branch
```

只有明确需要验证 Cookbook 构建时，才使用仓库外的临时构建目录：

```powershell
$cookbookBuild = Join-Path $env:TEMP 'huli-modern-vulkan-cookbook-build'
cmake -S $cookbookRoot -B $cookbookBuild
cmake --build $cookbookBuild
```

这些命令可能下载依赖并耗时较长；不得在 Cookbook 源码目录内生成 Huli 的笔记或实验状态。

## 文档验证

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```
