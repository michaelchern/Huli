# Huli AI 入口

> 本文件是根 AI 入口的中文源文件。修改根规则时，先改这里，再同步更新 `AGENTS.md`。
> 其他中文源位于 `docs/agents/zh-CN/`、`docs/tasks/zh-CN/` 和 `.agents/skills/*/SKILL.zh-CN.md`。
> 英文文件是 AI 默认读取入口，必须与中文源保持一致。

## 1. 核心原则

Huli 是一个 C++20 / Vulkan 学习仓库。根 CMake 已接入并可构建 Vulkan/图形第三方依赖；Huli 自身的应用目标与 Vulkan 运行入口尚未接入根构建。

AI 在本仓库工作时必须：

- 先读本文件，再按任务读取 `docs/agents/*.md`。
- 只加载当前任务需要的上下文，避免把 Vulkan、构建和 Git 流程一次性塞满上下文。
- 修改前查看 `git status --short --branch`。
- 不要回滚用户已有改动。
- 以当前 `CMakeLists.txt` 和源码目录为准，不要假设尚未接入根构建的教材、实验目标或运行入口已经存在。
- Huli 的学习产物应落在本仓库；引用外部材料时说明来源和学习目的，不要无说明地大段复制。
- 每次实质改动都给出验证命令和结果；纯文档改动通常只需要同步检查和 diff 检查。

## 2. 路由表

按任务只读需要的文件：

- 总索引和上下文选择：`docs/agents/index.md`
- 构建、CMake、Visual Studio、验证命令：`docs/agents/build.md`
- 本地提交、GitHub 发布、commit、PR：`docs/agents/git.md`
- 格式化、clang-format、clang-tidy：`docs/agents/formatting.md`
- codegraph、符号流、调用链、重构影响面：`docs/agents/codegraph.md`
- Vulkan 概念、调试、验证层、GPU 现象解释：`docs/agents/vulkan.md`
- 学习笔记、主题状态、理解/复现记录：`docs/agents/learning.md`

项目内共享 Agent skill：

- `.agents/skills/huli-workflow/SKILL.md`：仓库通用工作流、口令路由和上下文路由。

这些 skill 是普通 Markdown 工作流，不局限于 Codex；其他 AI Agent 也可以读取。

## 3. 关键目录

- `CMakeLists.txt`：根构建入口和实时依赖版本权威；当前可构建依赖目标，尚未接入 Huli 应用目标。
- `docs/agents/`：按需加载的 AI 上下文包。
- `docs/tasks/`：短小任务清单、主题学习状态、复现步骤和验证配方。
- `.agents/skills/`：项目共享 Agent skill。
- `tools/`：Agent 同步脚本和后续轻量工具。

## 4. 默认验证

修改 Agent 文档或 skill 后，必须检查同步：

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```

C++ / CMake / shader / 示例改动按需运行最小相关验证。当前根 CMake 可以验证第三方依赖，但这不等于 Huli 应用或 Vulkan 运行路径已经接入；汇报时明确实际验证范围。

纯文档改动通常不需要 CMake 构建。

## 5. 项目口令

这些口令使用 `=` 前缀，避免和 slash command、mention 语法冲突。

### `=sa`

根据中文源同步所有英文 Agent 文件。

- 不要修改中文源文件。
- 同步范围包括根 `AGENTS.md`、`docs/agents/*.md`、`docs/tasks/*.md` 和项目 skills。
- 保持章节结构一致；英文要短、直接、适合作为 AI 上下文。
- 更新对应英文文件顶部的 sync marker。
- 运行 `.\tools\sync-agents.ps1 -Check`。

### `=ca`

检查所有英文 Agent 文件是否与中文源同步。

- 只运行 `.\tools\sync-agents.ps1 -Check`。
- 不要修改文件。
- 检查范围由实际中文源动态发现，包括缺失目标、孤立英文文档或 skill、重复 marker 和明显未翻译的中文正文。
- 如果不同步，提示用户运行 `=sa`。

### `=ai`

把本轮或近期 AI 对话中值得长期保留的 Huli 学习上下文沉淀到项目 AI 资料中。

- 先查看 `git status --short --branch`，不要覆盖或回滚用户已有改动。
- 先判断材料是否值得沉淀：必须能在下一轮减少误判、缩短定位、明确禁止动作，或固定验证入口。
- 写入前搜索目标所有者和相关旧表述；优先更新或删除冲突事实，不要在旧结论旁追加一套新结论。
- 只沉淀稳定、可复用的内容：仓库规则、目录职责、学习流程、验证流程、常见误判和用户在本仓库内反复表达的偏好。
- 不要沉淀临时猜测、一次性命令输出、未定论争议、聊天闲谈、秘密信息、过窄的实现细节。
- 按归属选择目标：根规则写入 `AGENTS.zh-CN.md`；长期专项上下文写入 `docs/agents/zh-CN/*.md`；主题状态、复现步骤或验证配方写入 `docs/tasks/zh-CN/*.md`；共享工作流、口令、意图识别规则或跨 Agent 行为写入 `.agents/skills/huli-workflow/SKILL.zh-CN.md`。
- 可从当前代码或配置直接发现的依赖版本、目标和路径以实时文件为准，不在长期上下文中复制完整清单；一次性环境和验证结果写入带日期、命令和证据的 task 文档。
- 长任务恢复、证据、TODO 状态和失败探索默认使用 `docs/tasks/zh-CN/study-template.md`。只有用户明确要求文件规划工作流时，才可在被 Git 忽略的 `.planning/<plan-id>/` 中保存本机活动状态；完成后把稳定结论沉淀回 `docs/tasks/zh-CN/`。
- 写入前确认依据：用户明确偏好、当前代码事实、已验证命令、可靠外部来源或已落地设计；证据不足时只列候选，不写成规则。
- 修改中文源后，同步对应英文文件并更新 SHA256 marker。
- 如果没有足够确定、值得写入的内容，不要改文件，只汇报候选项和不沉淀的理由。
- 完成后运行 `.\tools\sync-agents.ps1 -Check` 并汇报结果。

### `=gc`

检查当前改动是否可以发布到 GitHub。

- 查看 `git status --short --branch`。
- 检查改动范围和相关 diff。
- 运行相关验证。
- 不要暂存、提交、推送或创建 PR。
- 如果准备好本地提交，提示用户可以运行 `=cm`。
- 如果用户明确需要 GitHub PR，再提示可以运行 `=gh`。

### `=cm`

只把当前意图明确的改动提交到当前本地分支。

- 先检查改动范围和验证结果。
- 只暂存本次任务相关文件，不要默认 `git add .`。
- commit 必须包含标题和中文正文摘要；正文说明改了什么、为什么改、验证了什么。
- 不要推送、不要创建 PR、不要合并分支。
- 汇报分支、commit 和验证结果。

### `=gh`

把当前意图明确的改动提交并发布到 GitHub PR。

- 先检查改动范围和验证结果。
- 只暂存本次任务相关文件，不要默认 `git add .`。
- 如果存在未提交改动，先按 `=cm` 规则提交。
- 推送当前分支到 `origin`。
- 创建 GitHub draft PR。
- 汇报分支、commit、PR URL 和验证结果。

## 6. 同步规则

中文文件是人类维护源，英文文件是 AI 默认读取入口。

同步关系：

- `AGENTS.zh-CN.md` -> `AGENTS.md`
- `docs/agents/zh-CN/*.md` -> `docs/agents/*.md`
- `docs/tasks/zh-CN/*.md` -> `docs/tasks/*.md`
- `.agents/skills/huli-workflow/SKILL.zh-CN.md` -> `.agents/skills/huli-workflow/SKILL.md`

每次修改任一中文源后，必须同步对应英文文件：

- 英文应清晰、简洁，适合 AI 作为入口上下文。
- 规则含义必须一致，不要求逐字翻译。
- 英文文件顶部的 SHA256 marker 必须匹配规范化为 UTF-8/LF 后的中文源。
- marker 证明英文文件对应哪个中文源版本，不替代对翻译含义的检查。
- 中文和英文冲突时，以中文为准。
- 不要在 `.agents/skills/` 下创建 `zh-CN/SKILL.md` 作为中文源，避免被 Agent 识别成第二个 skill。
