# Huli Learning Context
<!-- AGENT_DOCS_LEARNING_ZH_CN_SHA256: 1fda7b7d5115259c0a724519d6f2875571ffa909338dcd38b06cd3faf3122ab2 -->

Load this file only for learning plans, chapter notes, reproduction experiments, concept explanation, or long-lived state distillation.

## Learning Record Ownership

- Long-lived general rules: `AGENTS.zh-CN.md`
- Long-lived focused context: `docs/agents/zh-CN/*.md`
- Single-topic study state, reproduction steps, and validation records: `docs/tasks/zh-CN/*.md`
- Shared workflow and command recognition: `.agents/skills/huli-workflow/SKILL.zh-CN.md`

## Topic State Shape

For each study topic, prefer the shape in `docs/tasks/zh-CN/study-template.md`:

- Current facts
- Top next action
- Active items
- Evidence
- Failed explorations
- Validation

## Active Planning and Durable Knowledge

- `docs/tasks/zh-CN/` holds commit-worthy, reusable topic state, reproduction steps, and validation recipes.
- Do not create parallel state directories by default. When the user explicitly requests a file-planning workflow, Git-ignored `.planning/<plan-id>/task_plan.md`, `findings.md`, and `progress.md` may hold local active state.
- After an active plan completes, distill facts, validation entrypoints, and failed approaches that remain useful into the matching `docs/tasks/zh-CN/*.md`; do not commit the raw session log.
- Do not hardcode a user's local skill installation path in repository documentation.

## Teaching Style

- When the user says "I still do not understand", asks whether something is intuitive, or asks how it was invented, explain motivation and intuition first, then Vulkan API, math, or shader details.
- For graphics concepts, answer "what problem does it solve" first, state the reference source, then explain the smallest Huli reproduction.
- Study notes should be short, recoverable, and useful for the next hands-on step.
