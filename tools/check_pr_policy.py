#!/usr/bin/env python3
"""Validate Huli pull-request and commit subjects.

Usage:
    python3 ./tools/check_pr_policy.py \
        --title "docs(github): 精简单人提交门禁" \
        --base <base-ref-or-sha> [--head <head-ref-or-sha>]
"""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from dataclasses import dataclass


ALLOWED_TYPES = (
    "feat",
    "fix",
    "refactor",
    "perf",
    "docs",
    "test",
    "build",
    "ci",
    "style",
    "chore",
    "revert",
)
TYPE_PATTERN = "|".join(ALLOWED_TYPES)
SUBJECT_PATTERN = re.compile(
    rf"^(?P<type>{TYPE_PATTERN})"
    r"(?:\((?P<scope>[a-z0-9]+(?:-[a-z0-9]+)*)\))?"
    r"(?P<breaking>!)?: (?P<description>\S.*)$"
)
CHINESE_PATTERN = re.compile(r"[\u3400-\u4DBF\u4E00-\u9FFF]")
WIP_PATTERN = re.compile(r"^(?:wip|chore\(wip\))!?:", re.IGNORECASE)


@dataclass(frozen=True)
class CommitSubject:
    sha: str
    subject: str


def validation_error(subject: str) -> str | None:
    """Return the first policy error for a PR or commit subject."""

    if WIP_PATTERN.match(subject):
        return "WIP commits are local checkpoints and cannot be published"

    match = SUBJECT_PATTERN.fullmatch(subject)
    if match is None:
        return (
            "expected '<type>(<scope>)!: <Chinese description>' with an optional "
            "scope and breaking-change marker"
        )

    if not CHINESE_PATTERN.search(match.group("description")):
        return "description must contain at least one Chinese character"

    return None


def read_commit_subjects(base: str, head: str) -> list[CommitSubject]:
    """Read non-merge commit subjects in the half-open Git range base..head."""

    command = [
        "git",
        "log",
        "--no-merges",
        "--format=%H%x00%s",
        f"{base}..{head}",
    ]
    try:
        result = subprocess.run(
            command,
            check=True,
            capture_output=True,
            text=True,
        )
    except subprocess.CalledProcessError as error:
        detail = error.stderr.strip() or error.stdout.strip() or "git log failed"
        raise RuntimeError(detail) from error

    commits: list[CommitSubject] = []
    for line in result.stdout.splitlines():
        if not line:
            continue
        sha, separator, subject = line.partition("\0")
        if not separator:
            raise RuntimeError(f"Unexpected git log output: {line}")
        commits.append(CommitSubject(sha=sha, subject=subject))
    return commits


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Validate Huli PR and commit subjects before publication."
    )
    parser.add_argument("--title", required=True, help="Pull-request title to validate.")
    parser.add_argument(
        "--base",
        required=True,
        help="Base ref or commit excluded from the commit range.",
    )
    parser.add_argument(
        "--head",
        default="HEAD",
        help="Head ref or commit included in the commit range (default: HEAD).",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    failures: list[str] = []

    title_error = validation_error(args.title)
    if title_error:
        failures.append(f"PR title '{args.title}': {title_error}")

    try:
        commits = read_commit_subjects(args.base, args.head)
    except RuntimeError as error:
        print(f"Unable to inspect commit subjects: {error}", file=sys.stderr)
        return 2

    if not commits:
        failures.append(f"No non-merge commits found in {args.base}..{args.head}")

    for commit in commits:
        commit_error = validation_error(commit.subject)
        if commit_error:
            failures.append(
                f"Commit {commit.sha[:12]} '{commit.subject}': {commit_error}"
            )

    if failures:
        print("PR policy check failed:", file=sys.stderr)
        for failure in failures:
            print(f"- {failure}", file=sys.stderr)
        return 1

    print(
        f"PR title and {len(commits)} non-merge commit subject(s) pass Huli policy."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
