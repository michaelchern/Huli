<!--
PR 标题格式：<type>(<scope>): <中文简述>（scope 可省略）

允许的 type：
feat / fix / refactor / perf / docs / test / build / ci / style / chore / revert

示例：
fix(vulkan): 修复交换链重建后的同步状态
docs(agents): 统一提交与 PR 流程
-->

## 这次改了什么

### 摘要

<!-- 用简洁语言说明改了什么、为什么改；一个 PR 只解决一个清晰目标。 -->

### 变更类型

- [ ] `feat` 新功能或可观察能力
- [ ] `fix` 缺陷或验证错误修复
- [ ] `refactor` / `perf` 重构或性能优化
- [ ] `docs` / `test` 文档或测试
- [ ] `build` / `ci` / `style` / `chore` 工程维护
- [ ] `revert` 回滚

### 范围

- 关联 Issue / 学习任务：
- 本 PR 包含：
- 明确不包含：
- 用户可见变化：
- Breaking change：无 / 有，说明：

## 怎么验证的

### 自动验证

<!-- 列出实际执行的完整命令和结果，不要只写“已测试”。 -->

```text
命令：
结果：
```

### Vulkan Runtime Smoke

<!-- 涉及运行行为时填写；不涉及则写“不涉及”。编译链接成功不等于 runtime validation 通过。 -->

- 平台 / GPU：
- 启动命令：
- 结果：
- 首个 validation error / VUID：无 / 未执行 / 具体内容：

### 手工验证

<!-- 写明操作路径和观察结果；不涉及则写“不涉及”。 -->

### 未执行的验证

<!-- 写明未执行项、原因、未覆盖范围和风险；没有则写“无”。 -->

## 风险

### 风险分类

- [ ] 无已知风险
- [ ] Vulkan 对象生命周期 / 所有权 / 同步
- [ ] Runtime validation / 驱动 / GPU 差异
- [ ] CMake / 依赖 / toolchain / preset
- [ ] Windows / macOS 跨平台差异
- [ ] Shader / 模型 / 纹理 / 二进制资源
- [ ] 公共接口或行为兼容性
- [ ] 其他：

### 影响与回滚

- 影响范围：
- 回滚 / 降级方式：

<!-- 无风险时明确写“无”；存在 breaking change 或高风险改动时必须说明迁移与回滚方式。 -->

### 提交前检查

- [ ] 已 review 完整 diff 和未跟踪文件
- [ ] PR 只处理一个清晰目标
- [ ] commit 与 PR 标题符合 `<type>(<scope>): <中文简述>`
- [ ] 不包含 `chore(wip): ...` commit
- [ ] 未提交凭证、令牌、授权文件、个人数据或无关生成文件
- [ ] 已运行必要验证，或如实说明未执行项与风险
- [ ] Agent 文档或 skill 改动已通过同步检查
- [ ] 已补充必要文档和回滚说明
