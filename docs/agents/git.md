# Huli Git Workflow Context
<!-- AGENT_DOCS_GIT_ZH_CN_SHA256: 740bce71d7e4cfdfe0ce89cb021229aa5a6c5bd3f3678d749d2e6dcd49bde610 -->

Load this file only for `=br`, `=gc`, `=cm`, `=gh`, branches, commits, pushes, PRs, reviews, or publication checks.

This workflow references `makecindy/cindy` `main@9254c053` (2026-07-31), adapting its PR-first, Conventional Commit, validation-gate, risk-disclosure, and review ideas to Huli's C++20/Vulkan learning scope. It does not add DCO, automated code review, or new build CI.

## Delivery Principles

- Use PR-first delivery by default: code and documentation enter `main` through a PR from a non-default branch.
- Keep each branch, commit, and PR focused on one clear goal; exclude incidental fixes and unrelated generated files.
- Existing task branches, worktrees, and uncommitted changes take precedence. Do not move, stash, reset, clean, or revert existing work merely to fit the workflow.
- Do not push directly to `main`, mark a draft PR ready, merge a PR, or publish without explicit user authorization.
- Failed required validation blocks normal commits and publication. The only fallback is an explicitly requested local `chore(wip): ...` checkpoint, which must never be pushed or opened as a PR.

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
- `ci`: continuous integration or GitHub Actions configuration.
- `style`: formatting, source comments, or other non-logic style changes only.
- `chore`: repository maintenance not covered by another type.
- `revert`: reverting committed changes.
- `spike`: temporary learning experiments, technical validation, or prototypes; local only and not a publishable PR type.

Derive `description` from the purpose as concise English. It may contain only lowercase letters, digits, and single hyphens; it must not start or end with a hyphen or contain consecutive hyphens. The complete branch name must be at most 63 characters and pass `git check-ref-format --branch`. Do not use status, duplicate, or non-standard types such as `wip`, `add`, or `hotfix`; name the actual purpose instead.

Examples:

- `feat/vulkan-descriptor-pool`
- `fix/swapchain-resize`
- `build/macos-vulkan-env`
- `docs/contribution-workflow`
- `spike/descriptor-indexing`

## Commit and PR Subjects

Use this format for commit and PR subjects:

```text
<type>(<scope>): <Chinese short description>
```

- `scope` is optional. Use a stable English module or responsibility name such as `vulkan`, `render`, `example1`, `cmake`, `agents`, `docs`, or `github`.
- Allowed subject types are `feat`, `fix`, `refactor`, `perf`, `docs`, `test`, `build`, `ci`, `style`, `chore`, and `revert`.
- Write a concise Chinese description of the actual result. Do not substitute vague action words such as `add`, `update`, or `wip` for the type.
- Add `!` after the type or scope for a breaking change, then add a `BREAKING CHANGE:` section to the body describing impact and migration.
- `spike` is not a publishable subject type. When an experiment becomes deliverable, use the type that matches the final result.

Commit bodies must be written in Chinese and contain sections equivalent to:

```text
Changes:
- ...

Reason:
- ...

Verification:
- <command> — <result>
```

Add a risk section when compatibility, runtime, or rollback risk exists. Never claim an unexecuted check passed.

## Validation Gate

- Inspect the complete diff and untracked files before staging; run `git diff --cached --check` after staging.
- After changing agent documentation or skills, run the platform-appropriate sync check: `python3 ./tools/sync-agents.py --check` on macOS / Linux, or `.\tools\sync-agents.ps1 -Check` in Windows PowerShell. Confirm that the Chinese and English meanings agree.
- For C++, CMake, shader, example, or tool changes, run the smallest relevant formatting, configure, build, or test checks defined by `docs/agents/build.md` and `docs/agents/formatting.md`.
- Vulkan compilation and linking do not prove runtime correctness. Run a separate runtime smoke for runtime behavior changes and report the first validation error or VUID.
- If a highly relevant check cannot run, state why, what remains uncovered, and the risk. Whether a check is required depends on the change type and available platform.
- If required validation fails, both `=cm` and `=gh` stop. When the user explicitly requests a data-preservation or checkpoint commit, use `chore(wip): <Chinese short description>`, record the failed command and blocker in the body, and never push or open a PR from it.

## `=br <purpose>` Branch Creation

`=br <purpose>` or an explicit natural-language request to create a branch creates and switches branches. If the user only asks for a name or suggestion, return the suggestion without modifying the repository.

1. Run `git status --short --branch`; record the current branch, current `HEAD`, and uncommitted changes.
2. Inspect local branches and currently known remote refs; do not fetch automatically just to check a name.
3. Select the best matching type, turn the description into concise English kebab-case, and confirm the branch has one clear goal.
4. Check characters, length, and `git check-ref-format --branch`.
5. If the same local or remote ref already exists, stop and ask the user. Do not append `-2`, a date, or another suffix, and do not switch to the existing branch automatically.
6. Use the current `HEAD` as the default start point. Use `main`, a commit, or another branch only when the user explicitly specifies it.
7. Create and switch with `git switch -c <branch> [<start-point>]`. Carry uncommitted changes into the new branch unchanged; do not stash, reset, clean, or revert them.
8. Report the previous branch, new branch, start point, and whether uncommitted changes were carried. Do not commit, push, or create a PR.

## `=gc` Publication Precheck

Do not modify files, stage, commit, push, or create a PR.

1. Run `git status --short --branch` and confirm the current branch, upstream relation, and worktree state.
2. Inspect the complete diff and every untracked file. List the intended files and exclude unrelated changes, secrets, accidental generated files, and unnecessary binary assets.
3. Confirm the change serves one goal and propose a `<type>(<scope>): <Chinese short description>` commit/PR subject.
4. Run every required check and report each command, result, omitted check, and reason.
5. Identify compatibility, Vulkan lifetime/synchronization, runtime validation, CMake/dependency, cross-platform, and asset risks, including whether rollback is needed.
6. Conclude with one of: ready to commit, local WIP checkpoint only, or not ready to commit.
7. If ready for a normal local commit, mention `=cm`. Mention `=gh` only when the user explicitly needs a GitHub PR.

## `=cm` Local Commit

Do not push, create a PR, or merge branches.

1. Run `git status --short --branch` and inspect the complete diff, untracked files, and existing user changes.
2. Confirm the changes serve one clear goal. Stage only intended task files; do not default to `git add .`.
3. Inspect `git diff --cached` and run `git diff --cached --check` to ensure no unrelated work was included.
4. Run every required check. Any failed required validation stops the normal commit workflow.
5. Commit with a `<type>(<scope>): <Chinese short description>` subject and a Chinese body covering changes, reason, and verification. Add risk when needed and `BREAKING CHANGE:` for breaking changes.
6. Only an explicit request may create `chore(wip): ...` after validation failure; clearly mark it as non-publishable.
7. Stop after creating the local commit.
8. Report branch, commit hash, included files, and validation results.

## `=gh` Publish a Draft PR

1. Run `git status --short --branch` and inspect the complete diff, untracked files, and existing user changes.
2. The current branch must be a publishable non-default task branch. Stop on `main` or `spike/*` and require an appropriate task branch first.
3. If uncommitted changes exist, follow `=cm` to stage only target files, complete required validation, and create a local commit.
4. Confirm no commit being published uses `chore(wip): ...`, all required validation passed, and the changes serve one PR goal.
5. Use `<type>(<scope>): <Chinese short description>` for the PR title. For a multi-commit PR, summarize the overall result instead of copying the final commit subject mechanically.
6. Complete `.github/PULL_REQUEST_TEMPLATE.md` truthfully with summary, included/excluded scope, user-visible changes, actual checks, omitted checks, risks, and rollback.
7. Push the current branch to `origin` and open a GitHub draft PR.
8. Do not mark it ready, merge it, or push directly to `main` automatically. Those actions require separate explicit user authorization and passing required validation.
9. Report branch, commit hashes, PR URL, validation results, unverified scope, and primary risks.

## Review Severity

- `P0`: security issues, crashes, data corruption, hard Vulkan lifetime/synchronization errors, or definite cross-platform build/runtime failures. Fix before committing, publishing, or merging.
- `P1`: clear defects, regressions, rule violations, incomplete impact handling, or missing required validation. Fix in the current change.
- `P2`: style preferences or optional improvements. Do not block delivery; unless the user explicitly requests improvement suggestions, do not report these as review findings.

Read the applicable Huli rules and the complete diff before reviewing. Report only evidence-backed P0/P1 findings; do not substitute generic best practices for repository facts.

## Safety

- Never overwrite, revert, or stage unrelated worktree changes.
- Do not default to `git add .`.
- Do not commit credentials, tokens, authorization files, personal data, or unintended generated files.
- `=cm` never pushes, creates a PR, or merges branches.
- `=gh` creates a draft PR by default.
- Do not publish when required validation fails or a `chore(wip): ...` commit is present.
- Do not push directly to `main`, mark a draft PR ready, merge a PR, or publish without explicit user authorization.
