# Huli Codegraph Context

只在符号查找、调用链、重构影响面、架构理解或“哪里定义/谁调用”任务中加载本文件。

## 规则

- 先确认 `.understand-anything/knowledge-graph.json` 是否存在；若存在，再确认其项目路径和分析 commit 与当前 Huli 工作树匹配。
- 图不存在或已过期时，使用 `rg --files`、`rg`、CMake 目标和源码直接追踪，不要把缺失的图当成仓库故障。
- 只把当前仓库实际存在的代码当作 Huli 已实现行为；外部示例只能作为明确标注来源的参考。

## 常见问题

- “这个 Vulkan 对象在哪创建？”先查 Huli；如果仓库没有实现，直接说明当前缺失。
- “这个主题怎么复现？”先确认参考来源，再把最小实验写入 `docs/tasks/zh-CN/*.md`。
