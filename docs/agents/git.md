# Huli Git Workflow Context
<!-- AGENT_DOCS_GIT_ZH_CN_SHA256: a67ee4ae7e7ea7b1c6b43f74b969a3534fbd0752abd061b42150b189e840d860 -->

Load this file only for `=gc`, `=cm`, `=gh`, commits, pushes, PRs, or publication checks.

## `=gc` Check

Do not mutate anything.

1. Run `git status --short --branch`.
2. Inspect changed files and relevant diffs.
3. Inspect untracked files explicitly; `git diff --check` does not cover them.
4. Run `.\tools\sync-agents.ps1 -Check` when agent files exist.
5. Run the smallest relevant validation when C++, CMake, shader, or tool files changed.
6. Report which files would be included and whether validation passed.
7. If only a local commit is needed, tell the user to run `=cm`; mention `=gh` only when the user explicitly needs a PR.

## `=cm` Local Commit

Do not push, create a PR, or merge branches.

1. Run `git status --short --branch`.
2. Inspect changed files and relevant diffs before staging.
3. Stage only intended files; do not default to `git add .`.
4. After staging, run `git diff --cached --check` and relevant validation.
5. Commit with a concise title and a Chinese body summary.
6. The body must explain what changed, why it changed, and what was verified.
7. Stop after creating the local commit.
8. Report branch, commit hash, and validation result.

## `=gh` Publish

1. Run `git status --short --branch`.
2. Inspect changed files and relevant diffs before staging.
3. Stage only intended files; do not default to `git add .`.
4. After staging, run `git diff --cached --check` and relevant validation.
5. If uncommitted changes exist, create a local commit first using the `=cm` rules.
6. Push the current branch to `origin`.
7. Open a GitHub draft PR.
8. Report branch, commit hash, PR URL, and validation result.

## Safety

- Never push if validation fails unless the user explicitly asks to continue.
- Never overwrite unrelated worktree changes.
- `=cm` never pushes or creates a PR.
- `=gh` uses a draft PR by default.
