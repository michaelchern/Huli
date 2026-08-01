<p align="right">
  <a href="CONTRIBUTING.md">Simplified Chinese</a> · <strong>English</strong>
</p>

# Contributing to Huli

Thank you for contributing code, documentation, and learning records to Huli. Huli is a C++20/Vulkan learning repository; the live `CMakeLists.txt` and source tree are authoritative for build targets, dependency versions, and runtime entrypoints.

See `AGENTS.md` for detailed agent rules and `docs/agents/build.md` and `docs/agents/git.md` for build and Git workflows. This guide is the contributor entrypoint, not a replacement for those focused rules.

## Before You Start

- Run `git status --short --branch` and confirm the current branch, worktree, and existing changes.
- Do not overwrite, revert, stash, or accidentally submit somebody else's unfinished work.
- Do not commit credentials, tokens, authorization files, personal data, unrelated generated files, or accidental binary assets.
- Keep each branch and PR focused on one clear goal. Discuss larger compatibility, dependency, Vulkan lifetime/synchronization, or cross-platform changes before implementation.

## Branches

Use `<type>/<english-kebab-description>`, for example:

- `feat/vulkan-descriptor-pool`
- `fix/swapchain-resize`
- `build/macos-vulkan-env`
- `docs/contribution-workflow`

Branches should be short-lived. `spike/*` is only for local learning experiments and technical validation, not publishable PRs. Preserve an existing task branch, worktree, or uncommitted work instead of moving it merely to fit the workflow.

## Commits

Commit and PR subjects use:

```text
<type>(<scope>): <Chinese short description>
```

`scope` is optional and uses a concise English module name. Allowed types:

- `feat` / `fix` / `refactor` / `perf`
- `docs` / `test` / `build` / `ci`
- `style` / `chore` / `revert`

Add `!` after the type or scope for a breaking change and explain it in a `BREAKING CHANGE:` body section.

Normal local commit bodies are optional; the PR Description is the single complete record for scope, validation, risk, and rollback. A Chinese body is required for:

- Breaking changes: add `BREAKING CHANGE:` with impact and migration guidance.
- Local `chore(wip): ...` checkpoints: record the failed command and blocker, and never publish them.
- Material compatibility, Vulkan runtime, or rollback risk: describe the risk and mitigation.

Never claim an unexecuted check passed.

## Verification

Complete the checks required by the change before committing:

- Every change: review the complete diff and untracked files, then run `git diff --cached --check` after staging.
- Before publication: run `python3 ./tools/check_pr_policy.py --title "<PR title>" --base <base>` to check PR/commit subjects and unpublished WIP commits.
- Agent documentation or skills: run `python3 ./tools/sync-agents.py --check` on macOS / Linux, or `.\tools\sync-agents.ps1 -Check` in Windows PowerShell. Confirm that the Chinese and English meanings agree.
- C++, CMake, shader, example, or tool changes: follow `docs/agents/build.md` and `docs/agents/formatting.md` for relevant formatting, configure, build, or test checks.
- Vulkan runtime behavior: run a separate runtime smoke. Successful compilation and linking do not prove runtime validation; record the first validation error or VUID.

If a highly relevant check cannot run, explain why, the uncovered scope, and the risk in the PR. Failed required validation blocks normal commits and publication. Only an explicitly needed local data-preservation checkpoint may use `chore(wip): ...`; record the failed command and blocker in the body, and never push or open a PR from that commit.

GitHub `quality-gate` always checks subjects/WIP, the PR diff, Agent-document synchronization, and CMake preset parsing. It does not replace local builds or Vulkan runtime smoke.

## Pull Requests

Huli is PR-first by default. Do not push directly to `main`.

1. Submit from a publishable non-default task branch and keep the PR focused on one goal.
2. Use `<type>(<scope>): <Chinese short description>` for the PR title.
3. Review the complete diff and exclude credentials, unrelated generated files, accidental assets, and unrelated user changes.
4. Complete `.github/PULL_REQUEST_TEMPLATE.md` with the summary, included/excluded scope, user-visible behavior, actual checks, omitted checks, risks, and rollback.
5. Open a draft PR by default and wait for `quality-gate`. After required validation passes and the description is complete, a maintainer decides whether to mark it ready.
6. `main` uses Squash merge only. Do not merge, push directly to `main`, or publish without explicit authorization.

The remote repository enables Squash merge only, uses the PR Title and Description as the Squash subject and body, and deletes merged task branches automatically. `main` requires a PR and strict `quality-gate`, but solo maintenance does not require another person's approval.

## Review

- `P0`: security, crashes, data corruption, hard Vulkan lifetime/synchronization errors, or definite cross-platform build/runtime failures; must be fixed.
- `P1`: clear defects, regressions, rule violations, incomplete impact handling, or missing required validation; must be fixed in the current change.
- `P2`: style preferences or optional improvements; must not block delivery.

Base review findings on the current code, complete diff, and Huli rules rather than generic best practices.

## License

Huli-owned code is licensed under the MIT License in `LICENSE`. The project does not require DCO sign-off; do not add `Signed-off-by` or certify another person's contribution without a specific need.
