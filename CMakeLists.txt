cmake_minimum_required(VERSION 3.28)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project("Huli" VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#[[ 暂未接入 NRD；保留配置，等光线追踪路径需要 NRD 时恢复。
if (MSVC)
    # ShaderMake 对 NRD 要求的最低 Windows SDK 版本
    # 光线追踪项目需要使用 NRD
    set(_MIN_WINDOWS_SDK_VERSION_BUILD 20348)
endif()
]]

# 第三方依赖与 Vulkan SDK 集成。
find_package(Vulkan REQUIRED COMPONENTS SPIRV-Tools)
include(FetchContent)

# glslang：把 GLSL/HLSL 着色器源码编译为 SPIR-V。
# 该组件与已安装的 Vulkan SDK 保持一致，避免头文件与库版本不匹配。
if (Vulkan_VERSION VERSION_GREATER_EQUAL "1.4.350")
  set(ENABLE_OPT OFF CACHE BOOL "关闭 glslang 优化器集成" FORCE)
  FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG vulkan-sdk-${Vulkan_VERSION}.0
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(glslang)
  set(GLSLANG_LIB glslang)
else()
  find_package(Vulkan REQUIRED COMPONENTS SPIRV-Tools glslang)
  set(GLSLANG_LIB glslang)
endif()

include_directories(${Vulkan_INCLUDE_DIR})
message(STATUS "Vulkan_INCLUDE_DIR: ${Vulkan_INCLUDE_DIR}")
link_directories(${Vulkan_INCLUDE_DIR}/../lib)

# SPIRV-Reflect：读取 SPIR-V 字节码中的描述符绑定、着色器阶段和推送常量布局等元数据。
# 该组件与固定的 Vulkan SDK 版本保持一致。
set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "不构建 SPIRV-Reflect 可执行程序" FORCE)
set(SPIRV_REFLECT_EXAMPLES OFF CACHE BOOL "不构建 SPIRV-Reflect 示例" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "构建 SPIRV-Reflect 静态库" FORCE)
FetchContent_Declare(
  spirvreflect
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect.git
  GIT_TAG vulkan-sdk-1.4.350.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(spirvreflect)

# volk：在运行时加载 Vulkan 函数入口，避免手动解析每个 Vulkan 函数。
# 该组件与固定的 Vulkan SDK 版本保持一致。
FetchContent_Declare(
  volk
  GIT_REPOSITORY https://github.com/zeux/volk.git
  GIT_TAG vulkan-sdk-1.4.350.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(volk)

# Vulkan Memory Allocator：管理 Vulkan 设备内存的分配和子分配。
FetchContent_Declare(
  vma
  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
  GIT_TAG v3.4.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(vma)

# GLM：提供仅头文件的向量、矩阵、四元数和图形数学工具。
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

#[[ 当前 Vulkan 源码尚未使用 GLI；保留配置供纹理容器加载功能恢复。
# GLI：提供仅头文件的图像和 GPU 纹理容器加载工具。
set(GLI_TEST_ENABLE OFF CACHE INTERNAL "")
FetchContent_Declare(
  gli
  GIT_REPOSITORY https://github.com/g-truc/gli.git
  GIT_TAG 0.8.2.0
  GIT_SHALLOW TRUE
  SOURCE_SUBDIR _huli_skip_cmake
)
FetchContent_MakeAvailable(gli)
]]

#[[ 当前 Vulkan 源码直接通过平台 API 创建 Surface，尚未使用 ImGui 或 GLFW。
if (NOT ANDROID)
  # Dear ImGui：为工具界面和调试界面提供立即模式 UI 控件。
  FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.8
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR _huli_skip_cmake
  )
  FetchContent_MakeAvailable(imgui)
  add_library(
    imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
  )
  target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
  target_compile_definitions(imgui PUBLIC IMGUI_IMPL_VULKAN_NO_PROTOTYPES)

  # GLFW：创建桌面窗口，并提供键盘、鼠标和 Vulkan Surface 集成。
  set(GLFW_BUILD_DOCS OFF CACHE BOOL "不构建 GLFW 文档" FORCE)
  set(GLFW_BUILD_TESTS OFF CACHE BOOL "不构建 GLFW 测试" FORCE)
  set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "不构建 GLFW 示例" FORCE)
  FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(glfw)
endif()
]]

#[[ 当前 Vulkan 源码尚未使用 stb；保留配置供图像和字体加载功能恢复。
# stb：提供图像读取、图像写入、字体处理等单头文件工具。
# 上游没有正式发布标签，因此固定到已核对的不可变提交。
FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG 31c1ad37456438565541f4919958214b6e762fb4
  SOURCE_SUBDIR _huli_skip_cmake
)
FetchContent_MakeAvailable(stb)
]]

# Tracy：低开销 CPU/GPU 性能分析器，用于检查帧耗时和代码区间。
FetchContent_Declare(
  tracy
  GIT_REPOSITORY https://github.com/wolfpld/tracy.git
  GIT_TAG v0.13.1
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(tracy)

#[[ 当前 Vulkan 源码尚未使用 BS::thread_pool；保留配置供 CPU 任务并发功能恢复。
# BS::thread_pool：仅头文件的工作线程池，用于并发分发 CPU 任务。
FetchContent_Declare(
  threadpool
  GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
  GIT_TAG v5.1.0
  GIT_SHALLOW TRUE
  SOURCE_SUBDIR _huli_skip_cmake
)
FetchContent_MakeAvailable(threadpool)
]]

#[[ 当前 Vulkan 源码尚未包含文字渲染路径；FreeType 和 font-demo 一并保留待用。
# FreeType：把字体轮廓光栅化为文字渲染所需的字形位图。
set(FT_DISABLE_ZLIB ON CACHE BOOL "关闭 FreeType 的 zlib 支持" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "关闭 FreeType 的 bzip2 支持" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "关闭 FreeType 的 libpng 支持" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "关闭 FreeType 的 HarfBuzz 支持" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "关闭 FreeType 的 Brotli 支持" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "将第三方库构建为静态库" FORCE)
FetchContent_Declare(
  freetype
  GIT_REPOSITORY https://github.com/freetype/freetype.git
  GIT_TAG VER-2-14-3
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(freetype)

# font-demo：提供把 FreeType 字体轮廓转换为可渲染几何数据的轻量 C 工具。
# 上游没有正式发布标签，因此固定到已核对的不可变提交。
FetchContent_Declare(
  font-demo
  GIT_REPOSITORY https://github.com/kocsis1david/font-demo.git
  GIT_TAG 33b5358d2336e61d9db02b6073db7d6319863834
  SOURCE_SUBDIR _huli_skip_cmake
)
FetchContent_MakeAvailable(font-demo)
add_library(font-load-glyp "${font-demo_SOURCE_DIR}/geometry.c" "${font-demo_SOURCE_DIR}/outline.c")
add_definitions(-DFONT_DEMO_SOURCE_DIR="${font-demo_SOURCE_DIR}")
]]

#[[ 当前 Vulkan 源码尚未包含 glTF 资源加载路径；相关依赖和占位目标一并保留待用。
# RapidJSON：Microsoft glTF SDK 使用的仅头文件 JSON 解析器。
# 在这里提供接口目标，可以阻止 glTF-SDK 启动已不兼容 CMake 4.x 的旧式嵌套下载器。
# 此精确提交由 glTF-SDK 1.9.6 指定，并包含它需要的 SchemaDocument 接口；
# RapidJSON v1.1.0 对当前 glTF-SDK 来说过旧。
FetchContent_Declare(
  rapidjson_gltfsdk
  URL https://github.com/Tencent/rapidjson/archive/232389d4f1012dddec4ef84861face2d2ba85709.tar.gz
  URL_HASH SHA256=b9290a9a6d444c8e049bd589ab804e0ccf2b05dc5984a19ed5ae75d090064806
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
  SOURCE_SUBDIR _huli_skip_cmake
)
FetchContent_MakeAvailable(rapidjson_gltfsdk)
add_library(RapidJSON INTERFACE)
target_include_directories(RapidJSON INTERFACE ${rapidjson_gltfsdk_SOURCE_DIR}/include)

# Microsoft glTF SDK：读取、写入和校验 glTF 三维场景与资源文件。
# glTF-SDK 1.9.6 即使关闭测试也会探测 GoogleTest；这里用占位目标阻止旧式嵌套下载器，
# 同时不引入实际不会使用的测试代码。
if (NOT TARGET gtest)
  add_library(gtest INTERFACE)
  add_library(gtest_main INTERFACE)
  target_link_libraries(gtest_main INTERFACE gtest)
endif()
set(ENABLE_UNIT_TESTS OFF CACHE BOOL "不构建 glTF SDK 单元测试" FORCE)
set(ENABLE_SAMPLES OFF CACHE BOOL "不构建 glTF SDK 示例" FORCE)
set(RAPIDJSON_BUILD_DOC OFF CACHE BOOL "不构建 RapidJSON 文档" FORCE)
set(RAPIDJSON_BUILD_EXAMPLES OFF CACHE BOOL "不构建 RapidJSON 示例" FORCE)
set(RAPIDJSON_BUILD_TESTS OFF CACHE BOOL "不构建 RapidJSON 测试" FORCE)
FetchContent_Declare(
  gltfsdk
  GIT_REPOSITORY https://github.com/microsoft/glTF-SDK.git
  GIT_TAG r1.9.6.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(gltfsdk)
]]

#[[ 当前 Vulkan 源码尚未使用 meshoptimizer；保留配置供网格处理功能恢复。
# meshoptimizer：优化网格索引、顶点读取、过度绘制和网格压缩。
FetchContent_Declare(
  meshopt
  GIT_REPOSITORY https://github.com/zeux/meshoptimizer.git
  GIT_TAG v1.2
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(meshopt)
]]

#[[ 当前 Vulkan 源码尚未接入 DLSS；保留配置供超分辨率功能恢复。
# NVIDIA DLSS：提供 AI 辅助超分辨率所需的头文件和可再分发二进制文件。
FetchContent_Declare(
  dlss
  GIT_REPOSITORY https://github.com/NVIDIA/DLSS.git
  GIT_TAG v310.7.0
  GIT_SHALLOW TRUE
  SOURCE_SUBDIR _huli_skip_cmake
)
FetchContent_MakeAvailable(dlss)
set(DLSS_DLL "${dlss_SOURCE_DIR}/lib/Windows_x86_64/rel/nvngx_dlss.dll")
link_directories(${dlss_SOURCE_DIR}/lib/Windows_x86_64/x86_64/)
]]

#[[ 以下目录对应当前已注释的依赖，恢复依赖时再一并恢复。
include_directories(${stb_SOURCE_DIR})
]]
include_directories(${volk_SOURCE_DIR})
include_directories(${glm_SOURCE_DIR})
#[[
include_directories(${gli_SOURCE_DIR})
include_directories(${spirvcross_SOURCE_DIR})
]]
include_directories(${spirvreflect_SOURCE_DIR})
#[[
include_directories(${gltfsdk_SOURCE_DIR}/GLTFSDK/Inc)
if (NOT ANDROID)
  include_directories(${glfw_SOURCE_DIR}/include)
  include_directories(${imgui_SOURCE_DIR})
endif()
]]
include_directories(${tracy_SOURCE_DIR}/public)
#[[
include_directories(${threadpool_SOURCE_DIR}/include)
include_directories(${meshopt_SOURCE_DIR}/src)
include_directories(${freetype_SOURCE_DIR}/include)
include_directories(${font-demo_SOURCE_DIR})
include_directories(${dlss_SOURCE_DIR}/include)
]]

#[[ 当前没有目标使用 RequiredVulkanSDKLIBS，且其中的 SPIRV-Cross 尚未声明获取来源。
set(RequiredVulkanSDKLIBS 
    debug OSDependentd optimized OSDependent
    debug MachineIndependentd optimized MachineIndependent
    debug GenericCodeGend optimized GenericCodeGen
    debug glslangd optimized glslang
    debug SPIRVd optimized SPIRV
    debug SPIRV-Toolsd optimized SPIRV-Tools
    debug SPIRV-Tools-optd optimized SPIRV-Tools-opt
    debug glslang-default-resource-limitsd optimized glslang-default-resource-limits
    debug spirv-cross-cored optimized spirv-cross-core
    debug spirv-cross-glsld optimized spirv-cross-glsl
    debug spirv-cross-reflectd optimized spirv-cross-reflect)
if (GLSLANG_LIB)
  set(RequiredVulkanSDKLIBS 
      debug OSDependentd optimized OSDependent
      debug MachineIndependentd optimized MachineIndependent
      debug GenericCodeGend optimized GenericCodeGen
      debug glslangd optimized glslang
      debug SPIRVd optimized SPIRV
      debug SPIRV-Toolsd optimized SPIRV-Tools
      debug SPIRV-Tools-optd optimized SPIRV-Tools-opt
      debug glslang-default-resource-limitsd optimized glslang-default-resource-limits
      debug spirv-cross-cored optimized spirv-cross-core
      debug spirv-cross-glsld optimized spirv-cross-glsl
      debug spirv-cross-reflectd optimized spirv-cross-reflect)
endif()
]]

add_subdirectory(src/vulkan)
#add_subdirectory(source/enginecore)
#if(WIN32)
#add_subdirectory(source/chapter1)
#add_subdirectory(source/chapter2)
#add_subdirectory(source/chapter3)
#add_subdirectory(source/chapter4)
#add_subdirectory(source/chapter5)
#add_subdirectory(source/chapter6)
#add_subdirectory(source/chapter7)
#endif()
