# Huli Git Workflow Context

只在 `=br`、`=gc`、`=cm`、`=gh`、分支、提交、推送、PR、review 或发布检查任务中加载本文件。

本流程参考 `makecindy/cindy` 的 `main@9254c053`（2026-07-31），学习其 PR-first、Conventional Commit、验证门禁、风险披露和 review 思路，并按 Huli 的 C++20 / Vulkan 学习仓库边界调整。不引入 DCO、自动 Code Review 或新的构建 CI。

## 交付原则

- 默认 PR-first：代码和文档从非默认分支通过 PR 进入 `main`。
- 一个分支、commit 或 PR 只处理一个清晰目标，避免混入顺手修复和无关生成文件。
- 已有任务分支、worktree 和未提交改动优先保留；不要为了套流程而移动、stash、reset、clean 或回滚现有工作。
- 未经用户明确授权，不直推 `main`、把 draft PR 转为 ready、合并 PR 或执行发布。
- 必要验证失败时不得正常提交或发布；唯一兜底是用户明确要求的本地 `chore(wip): ...` 提交，该提交不得推送或创建 PR。

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
- `ci`：持续集成或 GitHub Actions 配置。
- `style`：只做格式化、源码注释或其他不改变逻辑的样式调整。
- `chore`：不属于以上类型的仓库维护。
- `revert`：回滚已提交的改动。
- `spike`：临时学习实验、技术验证或原型，仅限本地，不作为可发布 PR 类型。

`description` 必须由用途提炼成简洁英文，只包含小写字母、数字和单连字符；不能以连字符开头或结尾，不能包含连续连字符。完整分支名不得超过 63 个字符，并必须通过 `git check-ref-format --branch`。不要使用 `wip`、`add` 或 `hotfix` 等状态、重复或非标准类型；把实际工作目的写进名称。

示例：

- `feat/vulkan-descriptor-pool`
- `fix/swapchain-resize`
- `build/macos-vulkan-env`
- `docs/contribution-workflow`
- `spike/descriptor-indexing`

## Commit 与 PR 标题

commit 和 PR 标题统一使用：

```text
<type>(<scope>): <中文简述>
```

- `scope` 可省略；使用稳定的英文模块名或职责名，例如 `vulkan`、`render`、`example1`、`cmake`、`agents`、`docs` 或 `github`。
- 允许的标题 type 为 `feat`、`fix`、`refactor`、`perf`、`docs`、`test`、`build`、`ci`、`style`、`chore` 和 `revert`。
- 标题描述使用简洁中文，说明实际结果，不使用 `add`、`update`、`wip` 等模糊动作词充当类型。
- 破坏性变更在 type 或 scope 后添加 `!`，例如 `feat(render)!: 调整材质绑定接口`，并在正文增加 `BREAKING CHANGE:` 段说明影响与迁移方式。
- `spike` 不是可发布标题 type；实验结论要发布时，应按最终交付性质改用正式 type。

commit 正文必须使用中文并包含：

```text
变更：
- ...

原因：
- ...

验证：
- <命令> — <结果>
```

存在兼容性、运行时或回滚风险时，再增加“风险”一节。不要把未执行的验证写成已通过。

## 验证门禁

- 暂存前检查完整 diff 和未跟踪文件；暂存后运行 `git diff --cached --check`。
- 修改 Agent 文档或 skill 时运行平台适用的同步检查：macOS / Linux 使用 `python3 ./tools/sync-agents.py --check`，Windows PowerShell 使用 `.\tools\sync-agents.ps1 -Check`；并确认中英文含义一致。
- 修改 C++、CMake、shader、示例或工具时，按 `docs/agents/build.md` 和 `docs/agents/formatting.md` 运行最小相关格式、配置、编译或测试验证。
- 涉及 Vulkan 运行行为时，编译链接成功不等于运行验证通过；单独运行 runtime smoke，并报告首个 validation error / VUID。
- 某项高相关验证无法执行时，必须说明原因、未覆盖范围和风险；是否属于“必要验证”按改动类型与当前平台能力判断。
- 必要验证失败时，`=cm` 和 `=gh` 都必须停止。用户明确要求防丢数据或保留检查点时，可创建标题为 `chore(wip): <中文简述>` 的本地提交；正文必须记录失败命令和阻断原因，且不得 push 或创建 PR。

## `=br <用途>` 创建分支

`=br <用途>` 或明确要求“创建/新建分支”的自然语言请求会创建并切换分支。只要求“起名”“建议名称”或“这个用途叫什么分支”时，只返回建议，不修改仓库。

1. 运行 `git status --short --branch`，记录当前分支、当前 `HEAD` 和未提交改动。
2. 检查已有本地分支和当前已知的远程引用；不要为了命名检查自动 fetch。
3. 根据用途选择最匹配的 type，把描述提炼为简洁英文 kebab-case，并确认分支只处理一个清晰目标。
4. 检查字符、长度和 `git check-ref-format --branch`。
5. 如果同名本地或远程引用已存在，停止并询问用户；不要自动追加 `-2`、日期或其他后缀，也不要自动切换已有分支。
6. 默认使用当前 `HEAD` 作为基点；用户明确指定 `main`、commit 或其他分支时才使用该基点。
7. 使用 `git switch -c <branch> [<start-point>]` 创建并切换。未提交改动原样带入新分支；不要 stash、reset、clean 或回滚。
8. 创建后汇报原分支、新分支、基点和是否携带未提交改动。不要提交、推送或创建 PR。

## `=gc` 发布预检

不要修改文件、暂存、提交、推送或创建 PR。

1. 运行 `git status --short --branch`，确认当前分支、上游关系和工作区状态。
2. 检查完整 diff 和所有未跟踪文件，列出拟提交文件并排除无关改动、秘密信息、意外生成文件与不需要的二进制资源。
3. 确认改动只服务一个目标，并给出符合 `<type>(<scope>): <中文简述>` 的建议 commit / PR 标题。
4. 运行所有必要验证；逐项汇报命令、结果、未执行项及原因。
5. 识别兼容性、Vulkan 生命周期/同步、runtime validation、CMake/依赖、跨平台和资源文件风险，并说明是否需要回滚方案。
6. 给出“可提交”“仅可创建 WIP 检查点”或“尚不可提交”的结论。
7. 可正常本地提交时提示运行 `=cm`；只有用户明确需要 GitHub PR 时才提 `=gh`。

## `=cm` 本地提交

不要推送、创建 PR 或合并分支。

1. 运行 `git status --short --branch`，检查完整 diff、未跟踪文件和已有用户改动。
2. 确认当前改动只服务一个清晰目标；只暂存本次意图明确的文件，不要默认 `git add .`。
3. 暂存后检查 `git diff --cached` 和 `git diff --cached --check`，确认没有误纳无关改动。
4. 运行所有必要验证；任何必要验证失败都停止正常提交流程。
5. 使用 `<type>(<scope>): <中文简述>` 标题和“变更 / 原因 / 验证”中文正文提交；有风险时补充“风险”，有破坏性变更时补充 `BREAKING CHANGE:`。
6. 用户明确要求在验证失败时保存本地检查点，才可使用 `chore(wip): ...`；创建后明确标记为不可发布。
7. 创建本地提交后停止。
8. 汇报分支、commit hash、纳入文件和验证结果。

## `=gh` 发布 draft PR

1. 运行 `git status --short --branch`，检查完整 diff、未跟踪文件和已有用户改动。
2. 当前分支必须是可发布的非默认任务分支；如果位于 `main` 或 `spike/*`，停止并要求先使用合适的任务分支。
3. 如有未提交改动，按 `=cm` 规则只暂存目标文件、完成必要验证并创建本地提交。
4. 确认所有待发布 commit 都不是 `chore(wip): ...`，必要验证全部通过，改动只服务一个 PR 目标。
5. PR 标题使用 `<type>(<scope>): <中文简述>`；多 commit PR 按整体交付结果拟定，不机械复制最后一个 commit 标题。
6. 按 `.github/PULL_REQUEST_TEMPLATE.md` 如实填写摘要、包含/不包含范围、用户可见变化、实际验证、未验证项、风险和回滚方式。
7. 推送当前分支到 `origin`，创建 GitHub draft PR。
8. 不要自动转为 ready、合并 PR 或直推 `main`；这些动作需要用户另外明确授权，且必要验证必须通过。
9. 汇报分支、commit hash、PR URL、验证结果、未验证范围和主要风险。

## Review 严重度

- `P0`：安全问题、崩溃、数据损坏、Vulkan 生命周期/同步硬错误、确定的跨平台构建或运行失效。必须修复后才能提交、发布或合并。
- `P1`：明确缺陷、行为回归、规范违反、影响面处理不完整或缺失必要验证。必须在本次修复。
- `P2`：纯风格偏好或可选优化。不得阻塞交付；除非用户明确要求收集改进建议，否则不作为 review finding 上报。

Review 必须先读适用的 Huli 规则和完整 diff，再报告有证据支持的 P0 / P1；不要用泛化最佳实践替代仓库事实。

## 安全规则

- 不要覆盖、回滚或暂存无关工作树改动。
- 不要默认 `git add .`。
- 不要提交凭证、令牌、授权文件、个人数据或无意加入的生成文件。
- `=cm` 绝不推送、创建 PR 或合并分支。
- `=gh` 默认只创建 draft PR。
- 必要验证失败或存在 `chore(wip): ...` commit 时不得发布。
- 未经用户明确授权，不直推 `main`、把 draft PR 转为 ready、合并 PR 或执行发布。
