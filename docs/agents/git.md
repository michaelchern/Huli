# Huli Git Workflow Context
<!-- AGENT_DOCS_GIT_ZH_CN_SHA256: 750a6e3df5e3cc278a4db9812b8f32d35b61600b668583d869a8db2464e2074e -->

Load this file only for `=br`, `=gc`, `=cm`, `=gh`, branches, commits, pushes, PRs, or publication checks.

## Branch Naming Convention

Use `<type>/<english-kebab-description>` for every branch. Keep names vendor-neutral: do not add `codex/`, user names, or date prefixes.

Allowed `type` values:

- `feat`: new features, example capabilities, or observable behavior.
- `fix`: bug fixes, runtime errors, validation errors, or incorrect behavior.
- `docs`: documentation, agent material, or learning-record changes only.
- `refactor`: structural changes with no external behavior change.
- `perf`: performance or resource-usage improvements.
- `test`: tests, validation scripts, or test data.
- `build`: CMake, dependencies, toolchains, presets, or build environments.
- `style`: formatting, source comments, or other non-logic style changes only.
- `chore`: repository maintenance not covered by another type.
- `spike`: temporary learning experiments, technical validation, or prototypes.

Derive `description` from the purpose as concise English. It may contain only lowercase letters, digits, and single hyphens; it must not start or end with a hyphen or contain consecutive hyphens. The complete branch name must be at most 63 characters and pass `git check-ref-format --branch`. Do not use status, duplicate, or non-standard types such as `wip`, `add`, or `hotfix`; name the actual purpose instead.

Examples:

- `feat/vulkan-descriptor-pool`
- `fix/swapchain-resize`
- `build/macos-vulkan-env`
- `docs/branch-naming`
- `spike/descriptor-indexing`

## `=br <purpose>` Branch Creation

`=br <purpose>` or an explicit natural-language request to create a branch creates and switches branches. If the user only asks for a name or suggestion, return the suggestion without modifying the repository.

1. Run `git status --short --branch`; record the current branch, current `HEAD`, and uncommitted changes.
2. Inspect local branches and currently known remote refs; do not fetch automatically just to check a name.
3. Select the best matching `type` and turn the Chinese or natural-language purpose into concise English kebab-case.
4. Check characters, length, and `git check-ref-format --branch`.
5. If the same local or remote ref already exists, stop and ask the user. Do not append `-2`, a date, or another suffix, and do not switch to the existing branch automatically.
6. Use the current `HEAD` as the default start point. Use `main`, a commit, or another branch only when the user explicitly specifies it.
7. Create and switch with `git switch -c <branch> [<start-point>]`. Carry uncommitted changes into the new branch unchanged; do not stash, reset, clean, or revert them.
8. Report the previous branch, new branch, start point, and whether uncommitted changes were carried. Do not commit, push, or create a PR.

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
