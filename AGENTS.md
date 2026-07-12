# Huli Agent Entry
<!-- AGENTS_ZH_CN_SHA256: 2fb65ede7db0cddef52bdfd38095a1097ed9944fc51434a2e5f54618f917fad9 -->

> `AGENTS.zh-CN.md` is the Chinese source for the root AI entry.
> Other Chinese sources live in `docs/agents/zh-CN/`, `docs/tasks/zh-CN/`, and `.agents/skills/*/SKILL.zh-CN.md`.
> English files are the default AI entrypoints and must stay aligned with Chinese sources.

## 1. Core Rules

Huli is a C++20 / Vulkan learning repository. Its root CMake project builds Vulkan and graphics dependencies plus the `huli_vulkan` static library from `src/vulkan`; Huli application targets and the Vulkan runtime entrypoint are not yet connected.

When working in this repository:

- Read this file first, then load the relevant `docs/agents/*.md` file.
- Keep context narrow. Do not load Vulkan, build, and Git context all at once unless the task needs it.
- Check `git status --short --branch` before editing.
- Never revert user changes unless explicitly asked.
- Treat the live `CMakeLists.txt` and source tree as authoritative. Do not assume that study material, experiment targets, or runtime entrypoints have been connected to the root build.
- Keep Huli learning artifacts in this repository. When using external material, state its source and learning purpose instead of copying large sections without explanation.
- Report verification commands and results for every meaningful change. Docs-only changes normally need sync and diff checks only.

## 2. Context Router

Load only what the task needs:

- Index and context selection: `docs/agents/index.md`
- Build, CMake, Visual Studio, validation commands: `docs/agents/build.md`
- Local commits, GitHub publishing, commits, PRs: `docs/agents/git.md`
- Formatting, clang-format, clang-tidy: `docs/agents/formatting.md`
- Codegraph, symbol flow, call chains, refactor impact: `docs/agents/codegraph.md`
- Vulkan concepts, debugging, validation layers, GPU symptom explanation: `docs/agents/vulkan.md`
- Study notes, topic state, understanding and reproduction records: `docs/agents/learning.md`

Shared project agent skills:

- `.agents/skills/huli-workflow/SKILL.md`: repository workflow, project command routing, and context routing.

These skills are plain Markdown and vendor-neutral; they are not Codex-specific.

## 3. Key Paths

- `CMakeLists.txt`: root build entrypoint and live authority for dependency versions; it currently builds dependencies and `huli_vulkan`, but no Huli application target.
- `docs/agents/`: on-demand AI context packs.
- `docs/tasks/`: short task checklists, topic study state, reproduction steps, and validation recipes.
- `.agents/skills/`: shared project agent skills.
- `tools/`: agent sync scripts and future lightweight tools.

## 4. Default Validation

After changing agent docs or skills, check sync:

```powershell
.\tools\sync-agents.ps1 -Check
git diff --check
```

For C++, CMake, shader, or example changes, run the smallest relevant validation. The root project can validate dependencies and compile `huli_vulkan`, but that does not prove that a Huli application or Vulkan runtime path is connected. Report the exact validation scope.

Docs-only changes usually do not require a CMake build.

## 5. Project Commands

These commands use the `=` prefix to avoid conflicts with slash commands and mention syntax.

### `=sa`

Sync all English agent files from Chinese sources.

- Do not modify Chinese source files.
- Sync scope includes root `AGENTS.md`, `docs/agents/*.md`, `docs/tasks/*.md`, and project skills.
- Preserve the same section structure; English should stay short, direct, and AI-context friendly.
- Update sync markers near the top of matching English files.
- Run `.\tools\sync-agents.ps1 -Check`.

### `=ca`

Check whether all English agent files are synchronized with Chinese sources.

- Only run `.\tools\sync-agents.ps1 -Check`.
- Do not modify files.
- Discover scope from the actual Chinese sources and check missing targets, orphaned English documents or skills, duplicate markers, and obvious untranslated Chinese body text.
- If stale, tell the user to run `=sa`.

### `=ai`

Distill durable Huli learning context from this or recent AI conversations into the project AI materials.

- Run `git status --short --branch` first, and do not overwrite or revert existing user changes.
- First decide whether the material is worth preserving: it must reduce future misreads, shorten orientation, clarify forbidden actions, or lock in a validation entrypoint.
- Search the owning document and related old statements before writing. Update or remove conflicting facts instead of appending a second version beside them.
- Preserve only stable, reusable content: repository rules, directory responsibilities, learning workflow, validation workflows, common misreads, and recurring user preferences in this repo.
- Do not preserve temporary guesses, one-off command output, unresolved debates, casual chat, secrets, or overly narrow implementation details.
- Choose the target by ownership: root rules go in `AGENTS.zh-CN.md`; long-lived focused context goes in `docs/agents/zh-CN/*.md`; topic state, reproduction steps, or validation recipes go in `docs/tasks/zh-CN/*.md`; shared workflow, commands, intent-recognition rules, or cross-agent behavior go in `.agents/skills/huli-workflow/SKILL.zh-CN.md`.
- Treat live code and configuration as authoritative for discoverable dependency versions, targets, and paths; put one-off environments and validation results in task documents with dates, commands, and evidence.
- By default, use `docs/tasks/zh-CN/study-template.md` for long-task recovery, evidence, TODO state, and failed explorations. Only when the user explicitly requests a file-planning workflow may local active state live in Git-ignored `.planning/<plan-id>/`; distill stable results back into `docs/tasks/zh-CN/` afterward.
- Confirm the evidence before writing: explicit user preferences, current code facts, verified commands, reliable external sources, or settled designs. If evidence is weak, report candidates instead of turning them into rules.
- After changing a Chinese source, sync the matching English file and update the SHA256 marker.
- If nothing is certain or valuable enough to preserve, do not edit files; report candidates and why they were not preserved.
- Run `.\tools\sync-agents.ps1 -Check` and report the result.

### `=gc`

Check whether current changes are ready to publish to GitHub.

- Run `git status --short --branch`.
- Inspect changed files and relevant diffs.
- Run relevant validation.
- Do not stage, commit, push, or create a PR.
- If ready for a local commit, tell the user they can run `=cm`.
- Only mention `=gh` when the user explicitly needs a GitHub PR.

### `=cm`

Commit current intended changes to the current local branch only.

- Inspect scope and validation results first.
- Stage only files related to the current task; do not default to `git add .`.
- The commit must include a title and a Chinese body summary describing what changed, why it changed, and what was verified.
- Do not push, create a PR, or merge branches.
- Report branch, commit, and validation results.

### `=gh`

Commit current intended changes and publish them to a GitHub PR.

- Inspect scope and validation results first.
- Stage only files related to the current task; do not default to `git add .`.
- If uncommitted changes exist, commit them first using the `=cm` rules.
- Push the current branch to `origin`.
- Open a GitHub draft PR.
- Report branch, commit, PR URL, and validation results.

## 6. Sync Rule

Chinese files are the human-maintained sources. English files are the default AI entrypoints.

Sync relationships:

- `AGENTS.zh-CN.md` -> `AGENTS.md`
- `docs/agents/zh-CN/*.md` -> `docs/agents/*.md`
- `docs/tasks/zh-CN/*.md` -> `docs/tasks/*.md`
- `.agents/skills/huli-workflow/SKILL.zh-CN.md` -> `.agents/skills/huli-workflow/SKILL.md`

Whenever any Chinese source changes, update its matching English file:

- English should be clear, concise, and suitable as AI context.
- Preserve the same rule meanings; word-for-word translation is not required.
- The SHA256 marker near the top of the English file must match the Chinese source normalized to UTF-8/LF.
- A marker identifies the Chinese source revision; it does not replace checking translation meaning.
- If Chinese and English conflict, the Chinese file wins.
- Do not create `.agents/skills/*/zh-CN/SKILL.md` as a Chinese source; that can be discovered as a duplicate skill.
