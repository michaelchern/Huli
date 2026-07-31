#!/usr/bin/env python3
"""Discover and check synchronization between Chinese and English agent files.

Usage:
    python3 ./tools/sync-agents.py
    python3 ./tools/sync-agents.py --prompt
    python3 ./tools/sync-agents.py --check

The script is the macOS/Linux counterpart of ``tools/sync-agents.ps1``.
It does not modify repository files.
"""

from __future__ import annotations

import argparse
import hashlib
import re
import sys
from dataclasses import dataclass
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
CHINESE_PATTERN = re.compile(r"[\u3400-\u4DBF\u4E00-\u9FFF]")


@dataclass(frozen=True)
class SyncPair:
    source: str
    target: str
    marker: str
    label: str


@dataclass(frozen=True)
class SyncState:
    pair: SyncPair
    source_hash: str
    stored_hash: str
    target_exists: bool
    marker_count: int
    contains_chinese: bool

    @property
    def in_sync(self) -> bool:
        return (
            self.target_exists
            and self.marker_count == 1
            and self.stored_hash == self.source_hash
            and not self.contains_chinese
        )


def marker_slug(value: str) -> str:
    return re.sub(r"[^A-Za-z0-9]+", "_", value).strip("_").upper()


def repo_path(path: Path) -> str:
    resolved = path.resolve()
    try:
        return resolved.relative_to(REPO_ROOT).as_posix()
    except ValueError as error:
        raise ValueError(f"Path is outside the repository: {path}") from error


def normalized_sha256(path: Path) -> str:
    text = path.read_bytes().decode("utf-8-sig")
    normalized = text.replace("\r\n", "\n").replace("\r", "\n")
    return hashlib.sha256(normalized.encode("utf-8")).hexdigest()


def document_pairs(
    source_directory: str,
    target_directory: str,
    marker_prefix: str,
    label_prefix: str,
) -> list[SyncPair]:
    source_root = REPO_ROOT / source_directory
    if not source_root.is_dir():
        return []

    pairs: list[SyncPair] = []
    for source_path in sorted(source_root.glob("*.md"), key=lambda path: path.name):
        stem = source_path.stem
        pairs.append(
            SyncPair(
                source=repo_path(source_path),
                target=f"{target_directory}/{source_path.name}",
                marker=f"{marker_prefix}_{marker_slug(stem)}_ZH_CN_SHA256",
                label=f"{label_prefix} {stem}",
            )
        )
    return pairs


def skill_pairs() -> list[SyncPair]:
    skills_root = REPO_ROOT / ".agents/skills"
    if not skills_root.is_dir():
        return []

    pairs: list[SyncPair] = []
    for skill_directory in sorted(
        (path for path in skills_root.iterdir() if path.is_dir()),
        key=lambda path: path.name,
    ):
        source_path = skill_directory / "SKILL.zh-CN.md"
        if not source_path.is_file():
            continue
        skill_name = skill_directory.name
        pairs.append(
            SyncPair(
                source=repo_path(source_path),
                target=f".agents/skills/{skill_name}/SKILL.md",
                marker=f"{marker_slug(skill_name)}_SKILL_ZH_CN_SHA256",
                label=f"skill {skill_name}",
            )
        )
    return pairs


def discover_pairs() -> list[SyncPair]:
    pairs = [
        SyncPair(
            source="AGENTS.zh-CN.md",
            target="AGENTS.md",
            marker="AGENTS_ZH_CN_SHA256",
            label="root AGENTS",
        )
    ]
    pairs.extend(
        document_pairs("docs/agents/zh-CN", "docs/agents", "AGENT_DOCS", "agent context")
    )
    pairs.extend(
        document_pairs("docs/tasks/zh-CN", "docs/tasks", "TASK_DOCS", "task document")
    )
    pairs.extend(skill_pairs())
    return pairs


def orphaned_targets(known_targets: set[str]) -> list[str]:
    candidates: list[Path] = []
    for directory in (REPO_ROOT / "docs/agents", REPO_ROOT / "docs/tasks"):
        if directory.is_dir():
            candidates.extend(path for path in directory.glob("*.md") if path.is_file())

    skills_root = REPO_ROOT / ".agents/skills"
    if skills_root.is_dir():
        candidates.extend(path for path in skills_root.glob("*/SKILL.md") if path.is_file())

    return sorted(
        repo_path(path) for path in candidates if repo_path(path) not in known_targets
    )


def sync_state(pair: SyncPair) -> SyncState:
    source_path = REPO_ROOT / pair.source
    target_path = REPO_ROOT / pair.target
    if not source_path.is_file():
        raise FileNotFoundError(f"Missing source file: {pair.source}")

    source_hash = normalized_sha256(source_path)
    target_exists = target_path.is_file()
    target_text = target_path.read_bytes().decode("utf-8-sig") if target_exists else ""
    marker_pattern = re.compile(rf"<!-- {re.escape(pair.marker)}: ([a-fA-F0-9]{{64}}) -->")
    marker_matches = marker_pattern.findall(target_text)

    return SyncState(
        pair=pair,
        source_hash=source_hash,
        stored_hash=marker_matches[0].lower() if len(marker_matches) == 1 else "",
        target_exists=target_exists,
        marker_count=len(marker_matches),
        contains_chinese=bool(CHINESE_PATTERN.search(target_text)),
    )


def run_check(pairs: list[SyncPair]) -> int:
    states = [sync_state(pair) for pair in pairs]
    failed = [state for state in states if not state.in_sync]
    orphans = orphaned_targets({pair.target for pair in pairs})

    for state in failed:
        pair = state.pair
        if not state.target_exists:
            print(f"{pair.target} is missing for {pair.source}.")
            continue
        if state.marker_count == 0:
            print(f"{pair.target} has no sync marker for {pair.source}.")
        elif state.marker_count > 1:
            print(f"{pair.target} has duplicate sync markers for {pair.source}.")
        elif state.stored_hash != state.source_hash:
            print(f"{pair.target} has a stale source marker for {pair.source}.")
            print(f"Stored hash:  {state.stored_hash}")
            print(f"Current hash: {state.source_hash}")
        if state.contains_chinese:
            print(
                f"{pair.target} still contains Chinese text; "
                "finish the English synchronization."
            )

    for target in orphans:
        print(f"{target} has no matching Chinese source file.")

    if failed or orphans:
        return 1

    print(
        "All discovered English agent files have current source markers "
        "and pass English-content checks."
    )
    return 0


def build_prompt(pairs: list[SyncPair]) -> str:
    marker_lines = [
        f"- {pair.target}: <!-- {pair.marker}: {normalized_sha256(REPO_ROOT / pair.source)} -->"
        for pair in pairs
    ]
    pair_lines = [f"- {pair.source} -> {pair.target}" for pair in pairs]
    orphans = orphaned_targets({pair.target for pair in pairs})

    lines = [
        "Please synchronize the English AI-facing files from their Chinese source files.",
        "",
        "This is the `=sa` project command: sync all discovered agent context files.",
        "",
        "Rules:",
        "- Treat Chinese files as the source of truth.",
        "- Create a missing English target and remove or resolve orphaned "
        "English documents or skills.",
        "- Keep English files concise, direct, and free of untranslated Chinese text.",
        "- Preserve command names, paths, warnings, validation rules, and forbidden actions.",
        "- Preserve each file's purpose and section structure where practical.",
        "- Add or update exactly one matching marker in each English file:",
        *marker_lines,
        "- Do not modify Chinese source files unless explicitly asked.",
        "- Do not modify unrelated files.",
        "- Do not create `.agents/skills/*/zh-CN/SKILL.md`; use `SKILL.zh-CN.md` "
        "for Chinese skill sources to avoid duplicate skill discovery.",
        "",
        "Pairs:",
        *pair_lines,
        "",
        "After editing, run one platform-appropriate check:",
        "python3 ./tools/sync-agents.py --check",
        ".\\tools\\sync-agents.ps1 -Check",
    ]

    if orphans:
        lines.extend(
            ["", "Orphaned English files to resolve:", *(f"- {target}" for target in orphans)]
        )

    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument(
        "--check",
        action="store_true",
        help="check synchronization and return a nonzero status on failure",
    )
    mode.add_argument("--prompt", action="store_true", help="print the synchronization prompt")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    pairs = discover_pairs()
    if args.check:
        return run_check(pairs)
    print(build_prompt(pairs))
    return 0


if __name__ == "__main__":
    sys.exit(main())
