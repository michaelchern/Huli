add_library(
  huli_vulkan STATIC
  Buffer.cpp
  Buffer.hpp
  CommandQueueManager.cpp
  CommandQueueManager.hpp
  Common.cpp
  Common.hpp
  Context.cpp
  Context.hpp
  DynamicRendering.cpp
  DynamicRendering.hpp
  Framebuffer.cpp
  Framebuffer.hpp
  PhysicalDevice.cpp
  PhysicalDevice.hpp
  Pipeline.cpp
  Pipeline.hpp
  RenderPass.cpp
  RenderPass.hpp
  Sampler.cpp
  Sampler.hpp
  ShaderModule.cpp
  ShaderModule.hpp
  Swapchain.cpp
  Swapchain.hpp
  Texture.cpp
  Texture.hpp
  Utility.cpp
  Utility.hpp
  vma.cpp
)
add_library(Huli::Vulkan ALIAS huli_vulkan)

target_include_directories(
  huli_vulkan
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(
  huli_vulkan
  PUBLIC
    volk::volk
    GPUOpen::VulkanMemoryAllocator
    glm::glm
    glslang::glslang
  PRIVATE
    Tracy::TracyClient
    spirv-reflect-static
    glslang::glslang-default-resource-limits
)

target_compile_features(huli_vulkan PUBLIC cxx_std_20)
