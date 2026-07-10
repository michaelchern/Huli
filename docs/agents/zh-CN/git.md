# Huli Git Workflow Context

只在 `=gc`、`=cm`、`=gh`、提交、推送、PR 或发布检查任务中加载本文件。

## `=gc` 检查

不要修改任何文件。

1. 运行 `git status --short --branch`。
2. 检查改动文件和相关 diff。
3. 显式检查未跟踪文件；`git diff --check` 不会覆盖它们。
4. 如果存在 Agent 文件，运行 `.\tools\sync-agents.ps1 -Check`。
5. 如果改动 C++、CMake、shader 或工具，运行最小相关验证。
6. 汇报会纳入哪些文件，以及验证是否通过。
7. 如果只需要本地提交，提示用户运行 `=cm`；只有用户明确需要 PR 时才提 `=gh`。

## `=cm` 本地提交

不要推送、创建 PR 或合并分支。

1. 运行 `git status --short --branch`。
2. 暂存前检查改动文件和相关 diff。
3. 只暂存本次意图明确的文件；不要默认 `git add .`。
4. 暂存后运行 `git diff --cached --check` 和相关验证。
5. 使用简洁标题和中文正文摘要提交。
6. 正文说明改了什么、为什么改、验证了什么。
7. 创建本地提交后停止。
8. 汇报分支、commit hash 和验证结果。

## `=gh` 发布

1. 运行 `git status --short --branch`。
2. 暂存前检查改动文件和相关 diff。
3. 只暂存本次意图明确的文件；不要默认 `git add .`。
4. 暂存后运行 `git diff --cached --check` 和相关验证。
5. 如有未提交改动，先按 `=cm` 规则创建本地提交。
6. 推送当前分支到 `origin`。
7. 创建 GitHub draft PR。
8. 汇报分支、commit hash、PR URL 和验证结果。

## 安全规则

- 验证失败时不要推送，除非用户明确要求继续。
- 不要覆盖无关工作树改动。
- `=cm` 绝不推送或创建 PR。
- `=gh` 默认创建 draft PR。
