from __future__ import annotations

import sys
import unittest
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT))

from tools.check_pr_policy import validation_error  # noqa: E402


class SubjectPolicyTests(unittest.TestCase):
    def assert_valid(self, subject: str) -> None:
        self.assertIsNone(validation_error(subject), subject)

    def assert_invalid(self, subject: str) -> None:
        self.assertIsNotNone(validation_error(subject), subject)

    def test_accepts_scoped_chinese_subject(self) -> None:
        self.assert_valid("docs(github): 精简单人提交门禁")

    def test_accepts_breaking_subject(self) -> None:
        self.assert_valid("feat(render)!: 调整材质绑定接口")

    def test_rejects_non_conventional_subject(self) -> None:
        self.assert_invalid("Build/macos cmake presets")

    def test_rejects_description_without_chinese(self) -> None:
        self.assert_invalid("fix: update workflow")

    def test_rejects_publishable_wip_subject(self) -> None:
        self.assert_invalid("chore(wip): 保存未通过验证的改动")

    def test_rejects_plain_wip_subject(self) -> None:
        self.assert_invalid("WIP: 保存未完成改动")

    def test_rejects_invalid_scope(self) -> None:
        self.assert_invalid("docs(GitHub): 精简提交门禁")


if __name__ == "__main__":
    unittest.main()
