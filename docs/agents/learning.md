# Huli Learning Context
<!-- AGENT_DOCS_LEARNING_ZH_CN_SHA256: dd7166fc2cf2e047510225f32ec5b0abcaa61724a432765fc709ed3871f11a37 -->

Load this file only for learning methods, chapter understanding, reproduction experiments, concept explanation, or long-lived knowledge distillation.

## Learning Material Ownership

- Long-lived general rules: `AGENTS.zh-CN.md`
- Long-lived focused context: `docs/agents/zh-CN/*.md`
- Shared workflow and command recognition: `.agents/skills/huli-workflow/SKILL.zh-CN.md`

## Distillation Boundary

- Do not create repository task or state documents for individual study topics by default; keep active state in the current conversation.
- Update the matching `docs/agents/zh-CN/*.md` only with long-lived learning methods, common misreads, and reusable validation rules.
- Report one-off experiments, command output, temporary TODOs, and failed attempts only in the current task unless the user explicitly requests a document.
- Do not hardcode a user's local skill installation path in repository documentation.

## Teaching Style

- When the user says "I still do not understand", asks whether something is intuitive, or asks how it was invented, explain motivation and intuition first, then Vulkan API, math, or shader details.
- For graphics concepts, answer "what problem does it solve" first, state the reference source, then explain the smallest Huli reproduction.
- Study notes should be short, recoverable, and useful for the next hands-on step.
