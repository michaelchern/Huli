# Huli Formatting Context

只在格式化、clang-format、clang-tidy、编码或文本风格任务中加载本文件。

## 规则

- 修改前运行 `git status --short --branch`。
- 默认使用 UTF-8。中文文档和中文注释必须保持可读。
- `.gitattributes` 将 Markdown、PowerShell、Python、YAML 和 `.gitignore` 固定为 LF；不要用平台换行差异制造无意义 diff。
- 不要用格式化命令重写无关文件。
- 只格式化本次任务触碰的 C++ / shader / CMake 文件，除非用户明确要求全仓格式化。
- Agent 文档改动后运行 `.\tools\sync-agents.ps1 -Check` 和 `git diff --check`。
- `git diff --check` 不覆盖未跟踪文件；发布前必须显式检查未跟踪文件，暂存后再运行 `git diff --cached --check`。

## 当前状态

- `.clang-format` 和 `.clang-tidy` 目前是占位文件；不要假设已有完整风格策略。
- 如果要引入格式化策略，先把规则写清楚，再用最小范围验证。
