#pragma once

/// @file Common.hpp
/// @brief 提供 Vulkan 平台配置、结果检查、日志宏以及图像格式辅助函数。

// Volk 必须在包含头文件前知道目标窗口系统，才能声明对应的平台扩展。
#ifdef _WIN32
#if !defined(VK_USE_PLATFORM_WIN32_KHR)
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#endif

// 禁止 Vulkan 头文件声明静态函数原型，统一通过 Volk 加载的函数指针调用 Vulkan API。
#define VK_NO_PROTOTYPES
#include <volk.h>

// Windows 构建使用 Vulkan SDK 提供的枚举转字符串辅助函数输出可读错误信息。
#ifdef _WIN32
#include <vulkan/vk_enum_string_helper.h>
#endif

// 在包含 GLM 前启用其实验性扩展接口。
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

/// @def VK_CHECK
/// @brief 检查返回 `VkResult` 的 Vulkan 调用是否成功。
/// @param func 返回 `VkResult` 的表达式；宏只对该表达式求值一次。
/// @note 失败时会输出表达式、源文件、行号和结果；Windows 输出枚举名称，其他平台输出数值。
/// @warning 此宏通过 `assert(false)` 中止调试构建；禁用断言后只记录错误并继续执行，不会传播错误码。
/// @warning 宏展开为裸代码块，用于条件或循环分支时应由调用方显式添加花括号。
#ifdef _WIN32
#define VK_CHECK(func)                                                                                                 \
    {                                                                                                                  \
        const VkResult result = func;                                                                                  \
        if (result != VK_SUCCESS)                                                                                      \
        {                                                                                                              \
            std::cerr << "Error calling function " << #func << " at " << __FILE__ << ":" << __LINE__ << ". Result is " \
                      << string_VkResult(result) << std::endl;                                                         \
            assert(false);                                                                                             \
        }                                                                                                              \
    }
#else
#define VK_CHECK(func)                                                                                                 \
    {                                                                                                                  \
        const VkResult result = func;                                                                                  \
        if (result != VK_SUCCESS)                                                                                      \
        {                                                                                                              \
            std::cerr << "Error calling function " << #func << " at " << __FILE__ << ":" << __LINE__ << ". Result is " \
                      << result << std::endl;                                                                          \
            assert(false);                                                                                             \
        }                                                                                                              \
    }
#endif

/// @def LOGE
/// @brief 记录错误级别日志。
/// @param ... `printf` 风格的格式字符串及其参数。
/// @note Android 使用 `ANDROID_LOG_ERROR`；其他平台写入 `stderr`。
///
/// @def LOGW
/// @brief 记录警告级别日志。
/// @param ... `printf` 风格的格式字符串及其参数。
/// @note Android 使用 `ANDROID_LOG_WARN`；其他平台复用 LOGE()，不保留级别差异。
///
/// @def LOGI
/// @brief 记录信息级别日志。
/// @param ... `printf` 风格的格式字符串及其参数。
/// @note Android 使用 `ANDROID_LOG_INFO`；其他平台复用 LOGE()，不保留级别差异。
///
/// @def LOGD
/// @brief 记录调试级别日志。
/// @param ... `printf` 风格的格式字符串及其参数。
/// @note Android 使用 `ANDROID_LOG_DEBUG`；其他平台复用 LOGE()，不保留级别差异。
#ifdef __ANDROID__
// Android logcat 使用的固定标签；当前名称沿用现有实现。
#define TAG "OPENXR_SAMPLE"
#include <android/log.h>
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#else
#define LOGE(format, ...)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        fprintf(stderr, format, __VA_ARGS__);                                                                          \
        fprintf(stderr, "\n");                                                                                         \
    } while (0)
#define LOGW(format, ...) LOGE(format, __VA_ARGS__)
#define LOGI(format, ...) LOGE(format, __VA_ARGS__)
#define LOGD(format, ...) LOGE(format, __VA_ARGS__)
#endif

namespace Huli::Vulkan
{

    /// @brief 预留的渲染标记颜色，表示不透明红色 RGBA；当前 Vulkan 模块尚未使用该常量。
    constexpr glm::vec4 RENDER_COLOR{1.f, 0.f, 0.f, 1.0f};

    /// @brief 根据图像类型、创建标志和 multiview 状态选择对应的图像视图类型。
    /// @param[in] imageType Vulkan 图像的维度类型。
    /// @param[in] flags 图像创建标志；当前只检查 `VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT`。
    /// @param[in] multiview 是否将 1D 或 2D 图像视为数组视图。
    /// @return 与输入匹配的 `VkImageViewType`。
    /// @note 对 2D 图像而言，cube-compatible 标志优先于 @p multiview，并返回 `VK_IMAGE_VIEW_TYPE_CUBE`。
    /// @warning 未识别的 @p imageType 会触发断言；禁用断言时回退为 `VK_IMAGE_VIEW_TYPE_2D`。
    VkImageViewType imageTypeToImageViewType(VkImageType imageType, VkImageCreateFlags flags, bool multiview);

    /// @brief 查询当前实现支持的 Vulkan 格式每个 texel 所占的字节数。
    /// @param[in] format 要查询的 Vulkan 图像格式。
    /// @return 已列入静态映射的格式大小；`VK_FORMAT_UNDEFINED` 或未支持的格式返回 `0`。
    /// @note 此函数不查询物理设备能力；压缩、多平面及其他未列出的格式同样返回 `0`。
    uint32_t bytesPerPixel(VkFormat format);

}  // namespace Huli::Vulkan
