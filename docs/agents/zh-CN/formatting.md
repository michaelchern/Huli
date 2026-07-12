# Huli Formatting Context

只在格式化、clang-format、clang-tidy、编码或文本风格任务中加载本文件。

## 规则

- 修改前运行 `git status --short --branch`。
- 默认使用 UTF-8。中文文档和中文注释必须保持可读。
- `.gitattributes` 将 Git 判定为文本的文件统一为 LF，仅 `.bat` 和 `.cmd` 在工作区使用 CRLF；常见二进制与图形资源显式标记为 `binary`。不要用平台换行差异制造无意义 diff。
- `.editorconfig` 控制编辑器保存行为，`.gitattributes` 控制 Git 入库与检出规范化；两者必须保持相同的 LF / CRLF 策略。
- 修改 `.gitattributes` 后，用 `git check-attr` 验证代表路径，并用 `git ls-files --eol` 检查索引。只有在明确需要迁移已有换行且工作区干净时才运行 `git add --renormalize .`，并逐项审查暂存差异。
- 不要用格式化命令重写无关文件。
- 只格式化本次任务触碰的 C++ / shader / CMake 文件，除非用户明确要求全仓格式化。
- Agent 文档改动后运行 `.\tools\sync-agents.ps1 -Check` 和 `git diff --check`。
- `git diff --check` 不覆盖未跟踪文件；发布前必须显式检查未跟踪文件，暂存后再运行 `git diff --cached --check`。

## 当前状态

- `.clang-format` 和 `.clang-tidy` 目前是占位文件；不要假设已有完整风格策略。
- 如果要引入格式化策略，先把规则写清楚，再用最小范围验证。
