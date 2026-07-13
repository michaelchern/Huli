#pragma once

#include <string>

#include "Common.hpp"
#include "Utility.hpp"

/// @file Sampler.hpp
/// @brief 声明 Vulkan sampler 的 RAII 封装。

namespace Huli::Vulkan
{

    class Context;

    /// @brief 管理纹理采样参数对应的 `VkSampler`。
    ///
    /// 构造函数会创建并命名 Vulkan sampler。各向异性过滤固定关闭；`maxLod` 同时决定最大 mip level，
    /// 并控制 mipmap 过滤模式选择：大于 `0` 时使用线性过滤，否则使用最近点过滤。
    ///
    /// @note 本对象拥有创建得到的 `VkSampler`，并在析构时将其销毁。
    /// @note 本对象只保存非拥有的 `VkDevice` 句柄；对应逻辑设备必须比本对象存活更久。
    /// @warning 当前默认移动操作只会复制 Vulkan 句柄，不会清空源对象，可能导致重复销毁；
    ///          在实现安全的自定义移动操作前，不要移动 `Sampler` 对象。
    class Sampler final
    {
    public:
        MOVABLE_ONLY(Sampler);

        /// @brief 创建不启用深度比较的 Vulkan sampler。
        /// @param[in] context 提供逻辑设备和调试命名能力的上下文；其逻辑设备必须比本对象存活更久。
        /// @param[in] minFilter 期望用于纹理缩小操作的过滤方式。
        /// @param[in] magFilter 期望用于纹理放大操作的过滤方式。
        /// @param[in] addressModeU 超出 `[0, 1]` 范围时 U 坐标的寻址方式。
        /// @param[in] addressModeV 超出 `[0, 1]` 范围时 V 坐标的寻址方式。
        /// @param[in] addressModeW 超出 `[0, 1]` 范围时 W 坐标的寻址方式。
        /// @param[in] maxLod 允许访问的最大 mip level；大于 `0` 时同时启用线性 mipmap 过滤。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre @p context 必须包含有效的逻辑设备；所有参数必须满足 `VkSamplerCreateInfo` 的有效性要求。
        /// @warning 当前实现将 @p minFilter 写入 `VkSamplerCreateInfo::magFilter`，并将 @p magFilter
        ///          写入 `VkSamplerCreateInfo::minFilter`；此处仅记录现有行为，未修改实现。
        explicit Sampler(const Context& context,
                         VkFilter minFilter,
                         VkFilter magFilter,
                         VkSamplerAddressMode addressModeU,
                         VkSamplerAddressMode addressModeV,
                         VkSamplerAddressMode addressModeW,
                         float maxLod,
                         const std::string& name = "");

        /// @brief 创建支持可选深度比较的 Vulkan sampler。
        /// @param[in] context 提供逻辑设备和调试命名能力的上下文；其逻辑设备必须比本对象存活更久。
        /// @param[in] minFilter 期望用于纹理缩小操作的过滤方式。
        /// @param[in] magFilter 期望用于纹理放大操作的过滤方式。
        /// @param[in] addressModeU 超出 `[0, 1]` 范围时 U 坐标的寻址方式。
        /// @param[in] addressModeV 超出 `[0, 1]` 范围时 V 坐标的寻址方式。
        /// @param[in] addressModeW 超出 `[0, 1]` 范围时 W 坐标的寻址方式。
        /// @param[in] maxLod 允许访问的最大 mip level；大于 `0` 时同时启用线性 mipmap 过滤。
        /// @param[in] compareEnable 是否启用采样结果与参考值的比较。
        /// @param[in] compareOp 启用比较采样时使用的 Vulkan 比较操作。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre @p context 必须包含有效的逻辑设备；所有参数必须满足 `VkSamplerCreateInfo` 的有效性要求。
        /// @warning 当前实现将 @p minFilter 写入 `VkSamplerCreateInfo::magFilter`，并将 @p magFilter
        ///          写入 `VkSamplerCreateInfo::minFilter`；此处仅记录现有行为，未修改实现。
        explicit Sampler(const Context& context,
                         VkFilter minFilter,
                         VkFilter magFilter,
                         VkSamplerAddressMode addressModeU,
                         VkSamplerAddressMode addressModeV,
                         VkSamplerAddressMode addressModeW,
                         float maxLod,
                         bool compareEnable,
                         VkCompareOp compareOp,
                         const std::string& name = "");

        /// @brief 销毁本对象拥有的 Vulkan sampler。
        /// @pre 对应逻辑设备必须仍然有效，且所有引用该 sampler 的 GPU 操作必须已经结束。
        ~Sampler() { vkDestroySampler(device_, sampler_, nullptr); };

        /// @brief 获取底层 Vulkan sampler 句柄。
        /// @return 本对象拥有的 `VkSampler`；调用方不得销毁该句柄。
        VkSampler vkSampler() const { return sampler_; }
    private:
        /// 非拥有句柄；对应逻辑设备必须比本对象存活更久。
        VkDevice device_ = VK_NULL_HANDLE;

        /// 由本对象创建并销毁的 Vulkan sampler。
        VkSampler sampler_ = VK_NULL_HANDLE;
    };

}  // namespace Huli::Vulkan
