---
name: huli-workflow
description: Huli C++ Vulkan 学习仓库的通用 Agent 工作流。Agent 修改本仓库、处理 =sa/=ca/=ai/=br/=gc/=cm/=gh 口令，或需要路由到 branch、build、GitHub、formatting、Vulkan、learning 上下文时使用。
---

# Huli Workflow

本 skill 是普通 Markdown，Codex、Claude、Cursor、Gemini CLI 或其他 Agent 都可以读取。

## 开始

1. 阅读根目录 `AGENTS.md`。
2. 编辑前运行或查看 `git status --short --branch`。
3. 只加载相关的 `docs/agents/` 文件。
4. 改动范围保持贴合用户请求。
5. 汇报验证结果。

## 口令路由

- `=sa`：根据所有中文源同步英文 Agent 文件。
- `=ca`：只检查所有英文 Agent 文件是否与中文源同步。
- `=ai`：把近期 AI 对话中值得长期保留的 Huli 学习上下文沉淀到项目 AI 资料中。
- `=br <用途>`：根据用途创建并切换到规范命名的本地分支。
- `=gc`：只做 GitHub 发布预检。
- `=cm`：只提交目标改动到当前本地分支。
- `=gh`：提交目标改动并发布到 GitHub PR。

`=sa` 和 `=ca` 使用 `tools/sync-agents.ps1`；脚本动态发现中文源配对，并检查规范化 SHA256 marker、缺失目标、孤立英文文档或 skill，以及明显未翻译正文。

`=ai` 执行时：

1. 先查看 `git status --short --branch`，不要覆盖或回滚用户已有改动。
2. 从近期对话中提取可复用意图：用户触发词、正确入口、禁止动作、验证命令、已落地学习流程。
3. 搜索目标所有者和相关旧表述；如果新事实推翻旧事实，优先更新或删除旧表述，不要并列保留冲突版本。
4. 判断是否值得沉淀：内容必须能在下一轮减少误判、缩短定位、提高动手效率，或降低幻觉风险。
5. 按归属写入：
   - 根仓库规则：`AGENTS.zh-CN.md`。
   - 长期领域上下文：`docs/agents/zh-CN/*.md`。
   - 主题状态、复现步骤、验证配方：`docs/tasks/zh-CN/*.md`。
   - 共享工作流、口令、意图识别、跨 Agent 行为：`.agents/skills/huli-workflow/SKILL.zh-CN.md`。
6. 可从代码或配置直接发现的动态事实以实时文件为权威；一次性环境和验证结果写入带日期和证据的 task 文档，不复制到长期上下文。
7. 写入前确认事实依据来自用户明确偏好、当前代码、可靠外部来源、已验证命令或已落地设计。
8. 修改中文源后同步英文 AI 文件，并运行 `tools/sync-agents.ps1 -Check`。

执行 `=br` 时，或用户用自然语言明确要求创建分支时：

1. 先读取 `docs/agents/git.md`，并按其中的 `<type>/<english-kebab-description>` 规范生成名称。
2. 用户只要求起名或建议时，只返回名称，不修改仓库；只有明确要求创建时才创建并切换。
3. 检查工作区和已有引用，验证名称；同名引用存在时停止询问，不自动追加后缀或切换。
4. 默认从当前 `HEAD` 创建并保留未提交改动；不要 stash、reset、clean 或回滚。
5. 只创建和切换分支，不提交、推送或创建 PR。

`=gc`、`=cm` 和 `=gh` 执行前也读取 `docs/agents/git.md`。

## 上下文路由

- 构建或 CMake：`docs/agents/build.md`
- Codegraph / 符号流 / 调用链 / 影响面：`docs/agents/codegraph.md`
- GitHub publish / PR / commit：`docs/agents/git.md`
- 格式化 / 风格：`docs/agents/formatting.md`
- Vulkan 概念和调试：`docs/agents/vulkan.md`
- 学习笔记和主题状态：`docs/agents/learning.md`

如果上下文包不存在，直接检查源码，并在回答里说明假设。

## 源码注释与格式化意图

- 用户点名 `.hpp` 并要求“增加注释和格式化代码”时，先核对对应 `.cpp` 和调用位置，再增加中文 Doxygen 注释，重点说明公开职责、所有权、生命周期、同步、前置条件和当前实现边界；除非用户明确要求，不修改声明或代码逻辑。
- 用户点名 `.cpp` 并要求“格式化代码”“调整格式”或“不要修改代码”时，视为严格的 format-only 任务：只使用根 `.clang-format` 调整排版，不新增注释，不清理 include，不调整函数顺序，不重命名、重构或顺手修复逻辑；发现更深问题时只单独报告。
- 这类任务编辑前先检查目标文件 diff，避免覆盖用户已有改动；编辑后对目标文件运行 `clang-format --dry-run --Werror --style=file`、`git diff --check` 和换行检查，并用忽略注释或空白的内容对比确认代码保持不变。涉及公开头文件时再编译最小相关目标，并准确说明验证范围。

## 外部材料边界

- 当前没有固定教材或参考仓库，不要假设外部路径存在。
- 使用外部材料时说明来源、版本和学习目的。
- Huli 的实验、笔记和任务状态都写入 Huli 仓库。

## AI 资料和 Skill 设计

- Huli 保持薄路由：根 `AGENTS` 只放入口规则，长期领域上下文放 `docs/agents/zh-CN/`，短任务清单和主题学习状态放 `docs/tasks/zh-CN/`，skill 只承载强触发的工作流和意图识别。
- 不要整体照搬 Horizon 或其他仓库的 AI 框架。借鉴前先比较仓库目标、语言源、同步机制、上下文体量和过期风险。
- 多轮或长任务默认使用 `docs/tasks/zh-CN/study-template.md`。用户明确要求文件规划工作流时，可以把 `.planning/<plan-id>/` 作为被 Git 忽略的本机活动状态；完成后只把稳定结论沉淀到 `docs/tasks/zh-CN/`。
- 不要把本地安装的可选 skill 路径写入项目规则，也不要把会话流水账当作长期知识提交。
- 只有存在明确触发词、重复工作流或高频误判风险时才新增 skill；每个 skill 必须有准确的 frontmatter `description`。

## 硬规则

- 不要回滚用户改动，除非用户明确要求。
- 不要暂存无关文件。
- 不要默认 `git add .`。
- 验证失败时不要推送，除非用户明确要求继续。
- 不要修改用户未放入任务范围的外部仓库。
- 中文 skill 源文件使用 `SKILL.zh-CN.md`，不要放在 `zh-CN/SKILL.md`，避免被 Agent 识别为重复 skill。
