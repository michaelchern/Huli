# Huli Formatting Context
<!-- AGENT_DOCS_FORMATTING_ZH_CN_SHA256: a6a74f06efb222a1925c8e10dfe5c867bd5ff776a6cfbbb522982199bc6a72ab -->

Load this file only for formatting, clang-format, clang-tidy, encoding, or text-style work.

## Rules

- Run `git status --short --branch` before edits.
- Default to UTF-8. Chinese docs and Chinese comments must stay readable.
- `.gitattributes` normalizes every file Git detects as text to LF, except `.bat` and `.cmd`, which use CRLF in the worktree; common binary and graphics assets are explicitly marked `binary`. Do not create meaningless diffs from platform line-ending changes.
- `.editorconfig` controls how editors save files, while `.gitattributes` controls Git check-in and checkout normalization. Keep their LF / CRLF policies aligned.
- After changing `.gitattributes`, validate representative paths with `git check-attr` and inspect the index with `git ls-files --eol`. Run `git add --renormalize .` only when intentionally migrating existing line endings from a clean worktree, then review every staged change.
- Do not rewrite unrelated files with formatting commands.
- Format only C++ / shader / CMake files touched by the current task unless the user explicitly asks for a whole-repo format.
- After agent doc changes, run `.\tools\sync-agents.ps1 -Check` and `git diff --check`.
- `git diff --check` does not cover untracked files. Inspect untracked files explicitly before publication, then run `git diff --cached --check` after staging.

## Current State

- `.clang-format` and `.clang-tidy` are currently placeholder files. Do not assume a complete style policy exists.
- If adding a formatting policy, document the rule first and validate the smallest relevant scope.
