# Huli Formatting Context
<!-- AGENT_DOCS_FORMATTING_ZH_CN_SHA256: 1acbbb8d0287a281a1bb1bf826d494d74ab22cd523bf44cc448585b4f7195d03 -->

Load this file only for formatting, clang-format, clang-tidy, encoding, or text-style work.

## Rules

- Run `git status --short --branch` before edits.
- Default to UTF-8. Chinese docs and Chinese comments must stay readable.
- `.gitattributes` pins Markdown, PowerShell, Python, YAML, and `.gitignore` to LF. Do not create meaningless diffs from platform line-ending changes.
- Do not rewrite unrelated files with formatting commands.
- Format only C++ / shader / CMake files touched by the current task unless the user explicitly asks for a whole-repo format.
- After agent doc changes, run `.\tools\sync-agents.ps1 -Check` and `git diff --check`.
- `git diff --check` does not cover untracked files. Inspect untracked files explicitly before publication, then run `git diff --cached --check` after staging.

## Current State

- `.clang-format` and `.clang-tidy` are currently placeholder files. Do not assume a complete style policy exists.
- If adding a formatting policy, document the rule first and validate the smallest relevant scope.
