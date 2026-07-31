---
name: huli-workflow
description: Vendor-neutral workflow for AI agents working in the Huli C++ Vulkan learning repository. Use when an agent edits this repo, handles =sa/=ca/=ai/=br/=gc/=cm/=gh commands, or needs routing to branches, builds, GitHub, formatting, Vulkan, or learning context.
---
<!-- HULI_WORKFLOW_SKILL_ZH_CN_SHA256: d951c094ea6a0e28034a38aea41586900739767d1fc10ebaaaa1548840a84ed5 -->

# Huli Workflow

This skill is plain Markdown so Codex, Claude, Cursor, Gemini CLI, or other agents can use it.

## Start

1. Read root `AGENTS.md`.
2. Run or inspect `git status --short --branch` before edits.
3. Load only the relevant file from `docs/agents/`.
4. Keep changes scoped to the user request.
5. Report validation results.

## Command Routing

- `=sa`: sync all English agent files from Chinese sources.
- `=ca`: check whether all English agent files are synchronized with Chinese sources.
- `=ai`: distill durable Huli learning context from recent AI conversations into project AI materials.
- `=br <purpose>`: create and switch to a conventionally named local branch for the stated purpose.
- `=gc`: run GitHub publication precheck only.
- `=cm`: commit intended changes to the current local branch only.
- `=gh`: commit intended changes and publish them to a GitHub PR.

For `=sa` and `=ca`, use the platform-appropriate sync checker: `python3 ./tools/sync-agents.py --check` on macOS / Linux, or `.\tools\sync-agents.ps1 -Check` in Windows PowerShell. Both discover Chinese-source pairs dynamically and check normalized SHA256 markers, missing targets, orphaned English documents or skills, and obvious untranslated body text.

Git delivery follows this contract; `docs/agents/git.md` is authoritative:

- Use PR-first delivery by default. One branch and PR handle one clear goal, while existing task branches, worktrees, and uncommitted changes take precedence.
- Commit and PR subjects use `<type>(<scope>): <Chinese short description>` with optional scope. Commit bodies are in Chinese and state the changes, reason, and verification.
- `=gc` checks the complete diff, untracked files, intended scope, validation results, omitted checks, and risks.
- `=cm` stages only task files and creates a local commit only after required validation passes. A failed-validation checkpoint must be `chore(wip): ...` and cannot be published.
- `=gh` creates a draft PR only from a publishable non-default task branch and completes the PR template with scope, verification, risk, and rollback.

For `=ai`:

1. Inspect `git status --short --branch` first, and do not overwrite or revert existing user changes.
2. Extract reusable intent from recent conversations: user trigger phrases, the correct entrypoint, forbidden actions, validation commands, and settled learning workflows.
3. Search the owning document and related old statements. If a new fact supersedes an old one, update or remove the old statement instead of keeping conflicting versions.
4. Decide whether the material is worth preserving: it must reduce future misreads, shorten orientation, improve implementation speed, or lower hallucination risk.
5. Write to the right owner:
   - Root repository rules: `AGENTS.zh-CN.md`.
   - Long-lived domain context: `docs/agents/zh-CN/*.md`.
   - Topic state, reproduction steps, and validation recipes: `docs/tasks/zh-CN/*.md`.
   - Shared workflow, commands, intent recognition, and cross-agent behavior: `.agents/skills/huli-workflow/SKILL.zh-CN.md`.
6. Treat dynamic facts discoverable from code or configuration as live-file data. Put one-off environments and validation results in dated task documents with evidence instead of copying them into long-lived context.
7. Confirm evidence before writing: explicit user preferences, current code, reliable external sources, verified commands, or settled designs.
8. After changing a Chinese source, sync the English AI-facing file and run the platform-appropriate check: `python3 ./tools/sync-agents.py --check` on macOS / Linux, or `.\tools\sync-agents.ps1 -Check` in Windows PowerShell.

For `=br`, or when the user explicitly asks to create a branch in natural language:

1. Read `docs/agents/git.md` and derive a `<type>/<english-kebab-description>` name from the purpose.
2. If the user only asks for a name or suggestion, return the name without modifying the repository; create and switch only on an explicit creation request.
3. Inspect the worktree and known refs, then validate the name. If the ref already exists, stop and ask instead of appending a suffix or switching automatically.
4. Keep the branch short-lived and focused on one goal. `spike` is local experimentation only and cannot be published as a PR.
5. Create from the current `HEAD` by default and preserve uncommitted changes; do not stash, reset, clean, or revert them.
6. Only create and switch branches; do not commit, push, or create a PR.

For `=gc`, `=cm`, and `=gh`, also read `docs/agents/git.md` before acting.

## Context Routing

- Build or CMake: `docs/agents/build.md`
- Codegraph / symbol flow / call chains / impact: `docs/agents/codegraph.md`
- GitHub publish/PR/commit: `docs/agents/git.md`
- Formatting/style: `docs/agents/formatting.md`
- Vulkan concepts and debugging: `docs/agents/vulkan.md`
- Study notes and topic state: `docs/agents/learning.md`

If a context pack is missing, inspect source files directly and keep the answer explicit about assumptions.

## Source Documentation and Formatting Intent

- When the user names a `.hpp` file and asks to add comments and format it, inspect the matching `.cpp` and call sites first. Add Chinese Doxygen comments focused on public responsibilities, ownership, lifetime, synchronization, preconditions, and current implementation boundaries. Do not change declarations or code logic unless explicitly requested.
- When the user names a `.cpp` file and asks to format or adjust formatting, or says not to change code, treat it as a strict format-only task. Apply only the root `.clang-format`; do not add comments, clean up includes, reorder functions, rename, refactor, or opportunistically fix logic. Report deeper issues separately.
- Before these edits, inspect the target file's diff so existing user changes are not overwritten. Afterward, run target-scoped `clang-format --dry-run --Werror --style=file`, `git diff --check`, and line-ending checks, then compare content while ignoring comments or whitespace to confirm that code stayed unchanged. For public-header work, also compile the smallest relevant target and report the exact validation scope.

## External Material Boundary

- No textbook or reference repository is currently fixed. Do not assume an external path exists.
- State the source, version, and learning purpose when using external material.
- Huli experiments, notes, and task state go into the Huli repository.

## AI Material and Skill Design

- Keep Huli routing thin: root `AGENTS` only holds entry rules, long-lived domain context lives in `docs/agents/zh-CN/`, short task checklists and topic state live in `docs/tasks/zh-CN/`, and skills carry only strongly triggered workflows and intent recognition.
- Do not copy Horizon or other repository AI frameworks wholesale. Before borrowing, compare repository goal, language source, sync mechanism, context size, and drift risk.
- For multi-turn or long-running work, use `docs/tasks/zh-CN/study-template.md` by default. When the user explicitly requests a file-planning workflow, Git-ignored `.planning/<plan-id>/` may hold local active state; distill only stable results into `docs/tasks/zh-CN/` afterward.
- Do not write local optional-skill installation paths into project rules or commit raw session logs as long-lived knowledge.
- Add a new skill only when there is a clear trigger phrase, repeated workflow, or frequent-misread risk; every skill must have an accurate frontmatter `description`.

## Non-Negotiables

- Do not revert user changes unless explicitly asked.
- Do not stage unrelated files.
- Do not default to `git add .`.
- Do not normally commit or publish failed required validation. An explicitly authorized `chore(wip): ...` checkpoint stays local.
- Do not push directly to `main`, mark a draft PR ready, or merge a PR without explicit user authorization.
- Do not modify external repositories outside the user's requested scope.
- Use `SKILL.zh-CN.md` for Chinese skill sources; do not place Chinese sources at `zh-CN/SKILL.md`, because many agents discover every `SKILL.md` as a separate skill.
