# Huli Vulkan Context
<!-- AGENT_DOCS_VULKAN_ZH_CN_SHA256: 509755a7c90dd098e94cc1e228d0a2a32601e77cdfd1bd6d442ec99fa58a9e0a -->

Load this file only for Vulkan concepts, validation layers, GPU/driver issues, shaders, synchronization, descriptors, swapchains, or rendering-symptom explanations.

## Learning Posture

- Explain the purpose of the concept first, state the reference source, then say whether Huli currently implements it.
- If the user says they still do not understand, switch to intuition and visual cause-and-effect before Vulkan API details.
- Do not treat implementation constraints from external examples as Huli's long-term architecture rules. Huli may use smaller experiments for learning.

## Debugging Rules

- Find the first real error first: `VK_ERROR`, VUID, shader compile error, or MSVC `error C...`. Do not get distracted by wrapper logs.
- If logs come from another machine or another repository, state the evidence source and do not use Huli local state as reproduction proof.
- After shader / C++ / CMake changes, run the smallest relevant validation. If validation cannot run, explain why.

## External References

- No textbook or reference repository is currently fixed.
- After the user provides external material, record its source, version, file, and learning purpose.
- Treat external material as reference-only unless the user explicitly asks to bring it into Huli.
