# Huli Git Workflow Context

只在 `=br`、`=gc`、`=cm`、`=gh`、分支、提交、推送、PR 或发布检查任务中加载本文件。

## 分支命名规范

分支名统一使用 `<type>/<english-kebab-description>`，保持工具中立，不添加 `codex/`、用户名或日期前缀。

允许的 `type`：

- `feat`：新增功能、示例能力或可观察行为。
- `fix`：修复缺陷、运行错误、验证错误或不正确行为。
- `docs`：只修改文档、Agent 资料或学习记录。
- `refactor`：不改变外部行为的结构调整。
- `perf`：性能或资源使用优化。
- `test`：测试、验证脚本或测试数据。
- `build`：CMake、依赖、toolchain、preset 或构建环境。
- `style`：只做格式化、源码注释或其他不改变逻辑的样式调整。
- `chore`：不属于以上类型的仓库维护。
- `spike`：临时学习实验、技术验证或原型。

`description` 必须由用途提炼成简洁英文，只包含小写字母、数字和单连字符；不能以连字符开头或结尾，不能包含连续连字符。完整分支名不得超过 63 个字符，并必须通过 `git check-ref-format --branch`。不要使用 `wip`、`add` 或 `hotfix` 等状态、重复或非标准类型；把实际工作目的写进名称。

示例：

- `feat/vulkan-descriptor-pool`
- `fix/swapchain-resize`
- `build/macos-vulkan-env`
- `docs/branch-naming`
- `spike/descriptor-indexing`

## `=br <用途>` 创建分支

`=br <用途>` 或明确要求“创建/新建分支”的自然语言请求会创建并切换分支。只要求“起名”“建议名称”或“这个用途叫什么分支”时，只返回建议，不修改仓库。

1. 运行 `git status --short --branch`，记录当前分支、当前 `HEAD` 和未提交改动。
2. 检查已有本地分支和当前已知的远程引用；不要为了命名检查自动 fetch。
3. 根据用途选择最匹配的 `type`，把中文或自然语言描述提炼为简洁英文 kebab-case。
4. 检查字符、长度和 `git check-ref-format --branch`。
5. 如果同名本地或远程引用已存在，停止并询问用户；不要自动追加 `-2`、日期或其他后缀，也不要自动切换已有分支。
6. 默认使用当前 `HEAD` 作为基点；用户明确指定 `main`、commit 或其他分支时才使用该基点。
7. 使用 `git switch -c <branch> [<start-point>]` 创建并切换。未提交改动原样带入新分支；不要 stash、reset、clean 或回滚。
8. 创建后汇报原分支、新分支、基点和是否携带未提交改动。不要提交、推送或创建 PR。

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
