<div align="center">

<p align="center">
  <img src="docs/image/readme-hero.webp" alt="Huli | Vulkan Graphics Playground">
</p>

# Huli

<p align="center">
  <a href="./README.md">🇨🇳 中文</a> | <a href="./README.en.md">🇺🇸 English</a>
</p>

<p align="center">
  <a href="#project-goals">
    <img src="https://img.shields.io/badge/Graphics%20API-Vulkan-AC162C?style=flat-square&logo=vulkan&logoColor=white" alt="Vulkan">
  </a>
  <a href="#environment-and-build">
    <img src="https://img.shields.io/badge/C%2B%2B-20-00599C?style=flat-square&logo=cplusplus&logoColor=white" alt="C++20">
  </a>
  <a href="#environment-and-build">
    <img src="https://img.shields.io/badge/Platforms-Windows%20%7C%20macOS-555555?style=flat-square" alt="Windows and macOS">
  </a>
  <a href="LICENSE">
    <img src="https://img.shields.io/github/license/michaelchern/Huli?style=flat-square&color=green" alt="MIT License">
  </a>
</p>

<p align="center">
  <strong>From Vulkan Foundations to Modern Real-Time Rendering</strong>
</p>

</div>

---

A personal learning and experimentation repository for exploring modern Vulkan graphics with C++20. Starting with Vulkan infrastructure and a desktop example, the project progressively explores GPU-driven rendering, deferred shading, order-independent transparency, anti-aliasing, ray tracing, VR, and other modern graphics techniques.

- [🚀 Project Goals](#project-goals)
- [🗂️ Repository Structure](#repository-structure)
- [💻 Environment and Build](#environment-and-build)
- [📋 Module Progress](#module-progress)
- [📄 License and Notices](#license-and-notices)

## Project Goals

This repository aims to build a deeper understanding of the Vulkan API and modern graphics pipelines through hands-on implementation, with a focus on:

- **High-performance rendering pipelines**: programmable vertex pulling, multi-draw indirect, and bindless resource access
- **GPU-driven rendering**: moving scene management, visibility testing, and draw-command generation onto the GPU
- **Deferred rendering and screen-space effects**: deferred shading, SSAO, screen-space reflections, and shadows
- **Order-independent transparency (OIT)**: depth peeling, linked-list OIT, and weighted blended OIT
- **Anti-aliasing techniques**: practical exploration of MSAA, TAA, FXAA, and supersampling
- **Real-time ray tracing**: building a PBR rendering pipeline with the Vulkan ray-tracing extensions
- **Virtual reality rendering**: integrating OpenXR and exploring optimizations such as foveated rendering

Each experimental module is planned to be relatively independent and runnable on its own, providing a practical reference for continued learning and development.

## Repository Structure

> [!NOTE]
> The structure below remains a planning draft and may not match the live source tree. Treat the root `CMakeLists.txt` and the actual source tree as the authority for currently buildable targets.

```text
Huli/
├── modules/                  # Independent experimental modules
│   ├── 01_core_framework/    # Core framework: instance, device, swapchain, resources
│   ├── 02_vertex_pulling/    # Programmable vertex pulling and indirect drawing
│   ├── 03_gpu_driven/        # GPU-driven rendering and culling
│   ├── 04_deferred/          # Deferred rendering and screen-space effects
│   ├── 05_oit/               # Comparison of OIT techniques
│   ├── 06_antialiasing/      # Anti-aliasing experiments
│   ├── 07_raytracing/        # Ray-tracing pipeline
│   └── 08_openxr/            # VR/AR rendering and foveated rendering
├── shared/                   # Code, tools, and asset loading shared across modules
├── assets/                   # Models, textures, and other assets
├── notes/                    # Study notes, derivations, and practical findings
├── build/                    # Build output, excluded from version control
└── README.md
```

Each planned module will have its own `CMakeLists.txt` so it can be built and run independently.

## Environment and Build

### Development Environment

| Item | Environment |
| --- | --- |
| Language standard | C++20 |
| Operating systems | Windows 10 / Windows 11 / macOS |
| IDEs | Visual Studio Community / Visual Studio Code |
| Build tools | CMake 3.28+, Ninja Multi-Config |
| Compilers | MSVC, LLVM Clang, Apple Clang |
| Vulkan SDK | 1.4.350.0 recommended |
| Vulkan runtime on macOS | MoltenVK |
| Tested GPUs | NVIDIA RTX 4060 / GTX 1060 series |

### Prerequisites

- [Git](https://git-scm.com/)
- [CMake 3.28+](https://cmake.org/)
- [Ninja](https://ninja-build.org/)
- [Vulkan SDK](https://vulkan.lunarg.com/); version `1.4.350.0` is recommended
- Windows: the **Desktop development with C++** workload for Visual Studio 2022, or an available LLVM Clang toolchain
- macOS: Xcode Command Line Tools and a Vulkan SDK that includes MoltenVK

CMake downloads the required third-party dependencies during the initial configuration, so a working network connection is required.

### Clone the Repository

```bash
git clone https://github.com/michaelchern/Huli.git
cd Huli
```

### Windows (MSVC)

Run the following commands in Visual Studio Developer PowerShell:

```powershell
$env:VULKAN_SDK = "C:\VulkanSDK\1.4.350.0"

cmake --preset ninja-msvc
cmake --build --preset msvc-debug --target huli_example1 --parallel 8
.\out\build\ninja-msvc\examples\example1\Debug\huli_example1.exe
```

To use LLVM Clang, replace the configure preset with `ninja-clang` and use the `clang-debug`, `clang-release`, or `clang-relwithdebinfo` build preset.

### macOS

```bash
cmake --preset ninja-macos
cmake --build --preset macos-debug --target huli_example1 --parallel 8
./out/build/ninja-macos/examples/example1/Debug/huli_example1
```

### Other Build Configurations

The project also provides Release, RelWithDebInfo, MSVC AddressSanitizer, and Tracy presets. List all presets available on the current platform with:

```bash
cmake --list-presets=all
```

> [!IMPORTANT]
> Successful compilation and linking only prove that the build passes; they do not prove that Vulkan runtime validation passes. When running the example, monitor the validation-layer output and address the first VUID error before any later errors.

## Module Progress

The table below describes the planned learning path. It does not imply that every module is connected to the current root CMake project; refer to the source tree and `CMakeLists.txt` for the buildable targets.

| Module | Topic | Status | Runnable |
| :---: | --- | :---: | :---: |
| 01 | Core framework | ✅ | ✅ |
| 02 | Programmable vertex pulling | 🚧 | 🚧 |
| 03 | GPU-driven rendering | ⬜ | ⬜ |
| 04 | Deferred rendering | ⬜ | ⬜ |
| 05 | Order-independent transparency | ⬜ | ⬜ |
| 06 | Anti-aliasing experiments | ⬜ | ⬜ |
| 07 | Ray tracing | ⬜ | ⬜ |
| 08 | OpenXR and VR | ⬜ | ⬜ |

> **Legend**: ✅ Complete | 🚧 In progress | ⬜ Planned

## License and Notices

This repository is a personal learning and experimentation project intended to share practical experience with Vulkan graphics programming.

Original project code is licensed under the [MIT License](LICENSE) and may be used, modified, and distributed under its terms. Perform sufficient testing and validation before using it in production.

Third-party dependencies fetched through CMake, along with any community models, textures, or other assets referenced by the repository, remain subject to the licenses and terms specified by their respective authors and projects.

Explore, experiment, and share ideas. Let us see how far we can push the GPU.
