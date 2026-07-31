<p align="right">
  <strong>简体中文</strong> · <a href="CONTRIBUTING.en.md">English</a>
</p>

# Huli 贡献指南

感谢你为 Huli 贡献代码、文档和学习记录。Huli 是 C++20 / Vulkan 学习仓库；实时构建目标、依赖版本和运行入口以当前 `CMakeLists.txt` 与源码树为准。

详细 Agent 规则见 `AGENTS.md`，构建与 Git 流程分别见 `docs/agents/build.md` 和 `docs/agents/git.md`。本指南是贡献者入口，不替代这些专项规则。

## 开始之前

- 运行 `git status --short --branch`，确认当前分支、worktree 和已有改动。
- 不要覆盖、回滚、stash 或误提交他人的未完成工作。
- 不要提交凭证、令牌、授权文件、个人数据、无关生成文件或意外加入的二进制资源。
- 一个分支和 PR 只处理一个清晰目标；较大的兼容性、依赖、Vulkan 生命周期/同步或跨平台改动应先通过 issue 或讨论确认范围。

## 分支

分支名使用 `<type>/<english-kebab-description>`，例如：

- `feat/vulkan-descriptor-pool`
- `fix/swapchain-resize`
- `build/macos-vulkan-env`
- `docs/contribution-workflow`

分支应短生命周期。`spike/*` 只用于本地学习实验和技术验证，不作为可发布 PR 分支。已有任务分支、worktree 或未提交改动时优先保留现状，不要为了套流程强行迁移。

## Commit

标题和 PR 标题统一使用：

```text
<type>(<scope>): <中文简述>
```

`scope` 可省略，使用简洁英文模块名。允许的 type：

- `feat` / `fix` / `refactor` / `perf`
- `docs` / `test` / `build` / `ci`
- `style` / `chore` / `revert`

破坏性变更在 type 或 scope 后添加 `!`，并在正文增加 `BREAKING CHANGE:` 说明。

commit 正文使用中文，至少包含：

```text
变更：
- ...

原因：
- ...

验证：
- <命令> — <结果>
```

存在兼容性、运行时或回滚风险时补充“风险”一节。不要把未执行的检查写成已通过。

## 验证

提交前必须按改动范围完成必要验证：

- 所有改动：检查完整 diff、未跟踪文件，并在暂存后运行 `git diff --cached --check`。
- Agent 文档或 skill：macOS / Linux 运行 `python3 ./tools/sync-agents.py --check`，Windows PowerShell 运行 `.\tools\sync-agents.ps1 -Check`；确认中英文含义一致。
- C++ / CMake / shader / 示例 / 工具：按 `docs/agents/build.md` 与 `docs/agents/formatting.md` 运行相关格式、配置、编译或测试。
- Vulkan 运行行为：单独执行 runtime smoke；编译链接成功不等于运行验证通过，需记录首个 validation error / VUID。

高相关验证无法执行时，在 PR 中写明原因、未覆盖范围和风险。必要验证失败时不得正常提交或发布。只有明确需要本地防丢检查点时，才可使用 `chore(wip): ...`；正文记录失败命令和阻断原因，该提交不得 push 或创建 PR。

## Pull Request

Huli 默认 PR-first；不要直接向 `main` 推送。

1. 从可发布的非默认任务分支提交，确保 PR 只解决一个目标。
2. 使用 `<type>(<scope>): <中文简述>` 作为 PR 标题。
3. Review 完整 diff，排除凭证、无关生成文件、意外资源或不相关用户改动。
4. 按 `.github/PULL_REQUEST_TEMPLATE.md` 填写摘要、包含/不包含范围、用户可见变化、实际验证、未验证项、风险和回滚。
5. 默认创建 draft PR；必要验证通过并补全说明后，再由维护者决定是否转为 ready。
6. 等待 review；未经明确授权，不合并 PR、直推 `main` 或执行发布。

## Review

- `P0`：安全、崩溃、数据损坏、Vulkan 生命周期/同步硬错误、确定的跨平台构建或运行失效；必须修复。
- `P1`：明确缺陷、行为回归、规范违反、影响面不完整或缺失必要验证；必须在本次修复。
- `P2`：纯风格偏好或可选优化；不得阻塞交付。

Review 应以当前代码、完整 diff 和 Huli 规则为依据，不以泛化最佳实践替代证据。

## 许可说明

本仓库自有代码按 `LICENSE` 中的 MIT License 授权。本项目不要求 DCO sign-off；不要在没有明确需要时加入 `Signed-off-by` 或替他人声明贡献来源。
