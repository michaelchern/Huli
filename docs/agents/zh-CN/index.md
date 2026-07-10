# Huli Agent 上下文索引

按任务加载最小上下文。先读根 `AGENTS.md`，再按下面路由选择一个或少数几个文件。

## 路由

- 构建、CMake、Visual Studio、验证命令：`docs/agents/build.md`
- Git、提交、发布、PR：`docs/agents/git.md`
- 格式化、clang-format、clang-tidy：`docs/agents/formatting.md`
- codegraph、符号查找、调用链、影响面：`docs/agents/codegraph.md`
- Vulkan 概念、验证层、GPU 调试：`docs/agents/vulkan.md`
- 学习笔记、主题状态、复现记录：`docs/agents/learning.md`

## 默认流程

1. 运行或查看 `git status --short --branch`。
2. 判断任务是文档、学习、构建、Vulkan 调试还是 Git 发布。
3. 只加载对应上下文包。
4. 修改后给出验证命令和结果。

## 不要做

- 不要假设尚未加入仓库的源码、教材或构建目标已经存在。
- 使用外部材料时说明来源和学习目的，不要无说明地大段复制。
- 不要把临时聊天内容写成长期规则。
