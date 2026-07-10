# Huli Agent Context Index
<!-- AGENT_DOCS_INDEX_ZH_CN_SHA256: f8d8601e06939e117dc3a7d08353d3cf3f1df1b43b6aa521f080c86a85122c07 -->

Load the smallest useful context. Read root `AGENTS.md` first, then choose one or a few files from this router.

## Router

- Build, CMake, Visual Studio, validation commands: `docs/agents/build.md`
- Git, commits, publishing, PRs: `docs/agents/git.md`
- Formatting, clang-format, clang-tidy: `docs/agents/formatting.md`
- Codegraph, symbol lookup, call chains, impact: `docs/agents/codegraph.md`
- Vulkan concepts, validation layers, GPU debugging: `docs/agents/vulkan.md`
- Study notes, topic state, reproduction records: `docs/agents/learning.md`

## Default Flow

1. Run or inspect `git status --short --branch`.
2. Decide whether the task is docs, learning, build, Vulkan debugging, or Git publishing.
3. Load only the matching context pack.
4. After changes, report validation commands and results.

## Do Not

- Do not assume source material, study material, or build targets have already been added.
- State the source and learning purpose when using external material; do not copy large sections without explanation.
- Do not turn temporary chat content into long-lived rules.
