cmake_minimum_required(VERSION 3.28)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("Huli" VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # ShaderMake 对 NRD 要求的最低 Windows SDK 版本
    # 光线追踪项目需要使用 NRD
    set(_MIN_WINDOWS_SDK_VERSION_BUILD 20348)
endif()

find_package(Vulkan REQUIRED SPIRV-Tools)

include(FetchContent)

if (VulkanHeaderVersion2 GREATER_EQUAL 14350)
  FetchContent_Declare(glslang
      GIT_REPOSITORY https://github.com/KhronosGroup/glslang
      GIT_TAG vulkan-sdk-${Vulkan_VERSION}.0)
  if (NOT glslang_POPULATED)
    set(ENABLE_OPT OFF)
  endif()
  set(GLSLANG_LIB "glslang")
else()
  find_package(Vulkan REQUIRED SPIRV-Tools glslang)
endif()

include_directories(${Vulkan_INCLUDE_DIR})
message(STATUS "Vulkan_INCLUDE_DIR: ${Vulkan_INCLUDE_DIR}")
link_directories(${Vulkan_INCLUDE_DIR}/../lib)
