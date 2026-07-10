# Huli Codegraph Context
<!-- AGENT_DOCS_CODEGRAPH_ZH_CN_SHA256: deed6a223e6ef5073633154c27012a2c023859242da6dbed6a98be74049a5792 -->

Load this file only for symbol lookup, call chains, refactor impact, architecture understanding, or "where is this defined / who calls this" work.

## Rules

- First check whether `.understand-anything/knowledge-graph.json` exists. If it does, confirm its project path and analyzed commit match the current Huli worktree.
- If the graph is missing or stale, trace with `rg --files`, `rg`, CMake targets, and source directly. A missing graph is not a repository failure.
- Treat only code that exists in the current repository as implemented Huli behavior. External examples are references and must be labeled with their source.

## Common Questions

- "Where is this Vulkan object created?" Check Huli first. If it is not implemented, state that directly.
- "How should this topic be reproduced?" Confirm the reference source, then write the smallest Huli experiment into `docs/tasks/zh-CN/*.md`.
