# Huli Formatting Context

只在格式化、clang-format、clang-tidy、编码或文本风格任务中加载本文件。

## 规则

- 修改前运行 `git status --short --branch`。
- 默认使用 UTF-8。中文文档和中文注释必须保持可读。
- `.gitattributes` 将 Git 判定为文本的文件统一为 LF，仅 `.bat` 和 `.cmd` 在工作区使用 CRLF；常见二进制与图形资源显式标记为 `binary`。不要用平台换行差异制造无意义 diff。
- `.editorconfig` 控制编辑器保存行为，`.gitattributes` 控制 Git 入库与检出规范化；两者必须保持相同的 LF / CRLF 策略。
- `.clang-format` 是 C/C++ 语法格式的权威，`.editorconfig` 负责编辑器基础保存行为；两者的缩进和换行策略必须一致。
- C/C++ 使用 4 空格和 Allman 大括号；CMake 与 JSON 使用 2 空格。方法声明超过列宽后，每个参数独占一行，续行与左括号对齐。
- 修改 `.gitattributes` 后，用 `git check-attr` 验证代表路径，并用 `git ls-files --eol` 检查索引。只有在明确需要迁移已有换行且工作区干净时才运行 `git add --renormalize .`，并逐项审查暂存差异。
- 不要用格式化命令重写无关文件。
- 只格式化本次任务触碰的 C++ / shader / CMake 文件，除非用户明确要求全仓格式化。
- Agent 文档改动后运行 `.\tools\sync-agents.ps1 -Check` 和 `git diff --check`。
- `git diff --check` 不覆盖未跟踪文件；发布前必须显式检查未跟踪文件，暂存后再运行 `git diff --cached --check`。

## 当前状态

- 根 `.clang-format` 是首版 C++ 格式基线：120 列、4 空格、Allman 大括号、命名空间内容缩进，长参数列表不打包并与左括号对齐；不要在未审查 diff 时全仓格式化。
- 当前尚未建立 `.clang-tidy` 策略；不要假设已有静态检查规则。
- 后续调整格式策略时，先更新配置和本文件，再用代表性小样本验证输出，并避免把策略迭代混入无关代码改动。
