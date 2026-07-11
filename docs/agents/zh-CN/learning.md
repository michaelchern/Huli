# Huli Learning Context

只在学习计划、章节笔记、复现实验、概念解释或长期状态沉淀任务中加载本文件。

## 学习记录归属

- 长期通用规则：`AGENTS.zh-CN.md`
- 长期专项上下文：`docs/agents/zh-CN/*.md`
- 单个主题的学习状态、复现步骤、验证记录：`docs/tasks/zh-CN/*.md`
- 共享工作流和口令识别：`.agents/skills/huli-workflow/SKILL.zh-CN.md`

## 主题状态建议

每个学习主题优先使用 `docs/tasks/zh-CN/study-template.md` 的形状：

- 当前事实
- Top next action
- Active items
- Evidence
- Failed explorations
- Validation

## 活动规划与长期沉淀

- `docs/tasks/zh-CN/` 保存可提交、可复用的主题状态、复现步骤和验证配方。
- 默认不要创建平行状态目录。用户明确要求文件规划工作流时，可以使用被 Git 忽略的 `.planning/<plan-id>/task_plan.md`、`findings.md` 和 `progress.md` 保存本机活动状态。
- 活动计划完成后，把仍有长期价值的事实、验证入口和失败经验整理到对应 `docs/tasks/zh-CN/*.md`；不要直接提交会话流水账。
- 不要在仓库正式文档中硬编码某个用户的本地 skill 安装路径。

## 教学风格

- 用户表示“没看懂”“符合直觉吗”“怎么被发明的”时，先讲动机和直觉，再进入 Vulkan API、数学或 shader 细节。
- 对图形概念先回答“它解决什么问题”，再说明参考来源，最后回答“Huli 该怎么做最小复现”。
- 学习笔记要短、可恢复、能指导下一次动手。
