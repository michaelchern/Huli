# Huli Codegraph Context
<!-- AGENT_DOCS_CODEGRAPH_ZH_CN_SHA256: 2d856ff795fbe171818563c3b3dd2d64f2ed947967dbaf3ed3e67b20fb686ff2 -->

Load this file only for symbol lookup, call chains, refactor impact, architecture understanding, or "where is this defined / who calls this" work.

## Rules

- First check whether `.understand-anything/knowledge-graph.json` exists. If it does, confirm its project path and analyzed commit match the current Huli worktree.
- If the graph is missing or stale, trace with `rg --files`, `rg`, CMake targets, and source directly. A missing graph is not a repository failure.
- Treat only code that exists in the current repository as implemented Huli behavior. External examples are references and must be labeled with their source.

## Common Questions

- "Where is this Vulkan object created?" Check Huli first. If it is not implemented, state that directly.
- "How should this topic be reproduced?" Confirm the reference source, then provide the smallest Huli experiment and validation steps in the current answer; do not create a state document by default.
