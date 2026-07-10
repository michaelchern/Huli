---
name: huli-workflow
description: Vendor-neutral workflow for AI agents working in the Huli C++ Vulkan learning repository. Use when an agent edits this repo, handles =sa/=ca/=ai/=gc/=cm/=gh commands, or needs routing to build, GitHub, formatting, Vulkan, or learning context.
---
<!-- HULI_WORKFLOW_SKILL_ZH_CN_SHA256: a99abef49ec80c5f594de0a25fd1dcd777a6e00154056ae621d1a67dba437f5b -->

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
- `=gc`: run GitHub publication precheck only.
- `=cm`: commit intended changes to the current local branch only.
- `=gh`: commit intended changes and publish them to a GitHub PR.

For `=sa` and `=ca`, use `tools/sync-agents.ps1`. It discovers Chinese-source pairs dynamically and checks normalized SHA256 markers, missing targets, orphaned English files, and obvious untranslated body text.

For `=ai`:

1. Inspect `git status --short --branch` first, and do not overwrite or revert existing user changes.
2. Extract reusable intent from recent conversations: user trigger phrases, the correct entrypoint, forbidden actions, validation commands, and settled learning workflows.
3. Decide whether the material is worth preserving: it must reduce future misreads, shorten orientation, improve implementation speed, or lower hallucination risk.
4. Write to the right owner:
   - Root repository rules: `AGENTS.zh-CN.md`.
   - Long-lived domain context: `docs/agents/zh-CN/*.md`.
   - Topic state, reproduction steps, and validation recipes: `docs/tasks/zh-CN/*.md`.
   - Shared workflow, commands, intent recognition, and cross-agent behavior: `.agents/skills/huli-workflow/SKILL.zh-CN.md`.
5. Confirm evidence before writing: explicit user preferences, current code, reliable external sources, verified commands, or settled designs.
6. After changing a Chinese source, sync the English AI-facing file and run `tools/sync-agents.ps1 -Check`.

For `=gc`, `=cm`, and `=gh`, read `docs/agents/git.md` before acting.

## Context Routing

- Build or CMake: `docs/agents/build.md`
- Codegraph / symbol flow / call chains / impact: `docs/agents/codegraph.md`
- GitHub publish/PR/commit: `docs/agents/git.md`
- Formatting/style: `docs/agents/formatting.md`
- Vulkan concepts and debugging: `docs/agents/vulkan.md`
- Study notes and topic state: `docs/agents/learning.md`

If a context pack is missing, inspect source files directly and keep the answer explicit about assumptions.

## External Material Boundary

- No textbook or reference repository is currently fixed. Do not assume an external path exists.
- State the source, version, and learning purpose when using external material.
- Huli experiments, notes, and task state go into the Huli repository.

## AI Material and Skill Design

- Keep Huli routing thin: root `AGENTS` only holds entry rules, long-lived domain context lives in `docs/agents/zh-CN/`, short task checklists and topic state live in `docs/tasks/zh-CN/`, and skills carry only strongly triggered workflows and intent recognition.
- Do not copy Horizon or other repository AI frameworks wholesale. Before borrowing, compare repository goal, language source, sync mechanism, context size, and drift risk.
- When multi-turn or long-running tasks need recoverable state, use `docs/tasks/zh-CN/study-template.md`; do not create a parallel state directory by default.
- Add a new skill only when there is a clear trigger phrase, repeated workflow, or frequent-misread risk; every skill must have an accurate frontmatter `description`.

## Non-Negotiables

- Do not revert user changes unless explicitly asked.
- Do not stage unrelated files.
- Do not default to `git add .`.
- Do not push failed validation unless the user explicitly asks to continue.
- Do not modify external repositories outside the user's requested scope.
- Use `SKILL.zh-CN.md` for Chinese skill sources; do not place Chinese sources at `zh-CN/SKILL.md`, because many agents discover every `SKILL.md` as a separate skill.
