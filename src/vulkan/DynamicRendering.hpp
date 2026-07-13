#pragma once

#include "Common.hpp"
#include "Utility.hpp"
#include "vk_mem_alloc.h"

/// @file DynamicRendering.hpp
/// @brief 声明 Vulkan dynamic rendering 命令记录辅助工具。

namespace Huli::Vulkan
{
    /// @brief 记录 dynamic rendering 的开始、结束命令以及单张颜色图像的布局转换。
    ///
    /// 本类不保存状态，也不拥有任何 Vulkan 资源。所有函数只向调用方提供的命令缓冲区
    /// 记录命令，不负责提交、队列同步或等待 GPU 执行完成。
    ///
    /// @pre 逻辑设备必须启用 dynamic rendering feature，并满足所用 Vulkan 版本或扩展要求。
    /// @warning 内置 image barrier 只处理传入的单张颜色图像，并固定作用于 color aspect、
    ///          mip `0` 和 array layer `0`；depth、stencil、resolve 及其他子资源必须由调用方处理。
    class DynamicRendering final
    {
    public:
        /// @brief 描述一个 dynamic rendering attachment 及其 load、store 和 resolve 行为。
        /// @warning 没有默认值的成员必须由调用方在使用前完整初始化。
        struct AttachmentDescription
        {
            /// rendering 期间使用的 attachment 图像视图。
            VkImageView imageView;

            /// attachment 在 rendering 期间实际使用的图像布局。
            VkImageLayout imageLayout;

            /// resolve 操作模式；`VK_RESOLVE_MODE_NONE` 表示不执行 resolve。
            VkResolveModeFlagBits resolveModeFlagBits = VK_RESOLVE_MODE_NONE;

            /// resolve 目标图像视图；不执行 resolve 时保持 `VK_NULL_HANDLE`。
            VkImageView resolveImageView = VK_NULL_HANDLE;

            /// resolve 目标在 rendering 期间使用的图像布局。
            VkImageLayout resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            /// rendering 开始时对 attachment 内容执行的 load 操作。
            VkAttachmentLoadOp attachmentLoadOp;

            /// rendering 结束时对 attachment 内容执行的 store 操作。
            VkAttachmentStoreOp attachmentStoreOp;

            /// 当 attachmentLoadOp 为 `VK_ATTACHMENT_LOAD_OP_CLEAR` 时使用的清除值。
            VkClearValue clearValue;
        };

        /// @brief 获取当前辅助实现声明的实例扩展名称。
        /// @return 字符串 `VK_KHR_get_physical_device_properties2`。
        /// @note 此返回值本身不会启用 dynamic rendering 的设备扩展或 feature。
        static std::string instanceExtensions();

        /// @brief 可选转换一张颜色图像的布局，然后记录 `vkCmdBeginRendering`。
        /// @param[in] commandBuffer 用于记录命令的命令缓冲区。
        /// @param[in] image 在 @p oldLayout 与 @p newLayout 不同时执行布局转换的颜色图像。
        /// @param[in] renderingFlags 传递给 `VkRenderingInfo::flags` 的 rendering 标志。
        /// @param[in] rectRenderSize rendering 的渲染区域。
        /// @param[in] layerCount rendering 的层数。
        /// @param[in] viewMask multiview 使用的视图掩码；`0` 表示未启用 multiview。
        /// @param[in] colorAttachmentDescList 按 location 顺序排列的颜色 attachment 描述列表。
        /// @param[in] depthAttachmentDescList 可选的深度 attachment 描述；为空表示不使用深度 attachment。
        /// @param[in] stencilAttachmentDescList 可选的模板 attachment 描述；为空表示不使用模板 attachment。
        /// @param[in] oldLayout @p image 在可选 barrier 之前的布局。
        /// @param[in] newLayout @p image 在可选 barrier 之后的布局。
        /// @pre @p commandBuffer 必须处于可记录 graphics 命令的录制状态，且不得处于另一段 rendering 内。
        /// @pre 当 @p oldLayout 与 @p newLayout 不同时，@p image 必须有效并支持指定的颜色图像布局转换。
        /// @pre 所有 attachment 描述必须满足 Vulkan dynamic rendering 的格式、用法、布局、采样数和 resolve 要求。
        /// @note attachment 描述只在函数调用期间读取，函数返回后不保留其地址。
        /// @note 当布局不同时，当前实现会在 `vkCmdBeginRendering` 前记录一个从
        ///       `VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT` 到 `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` 的 barrier。
        /// @warning attachment 中记录的 imageLayout 必须与 barrier 后的实际布局一致；函数不会验证二者是否匹配。
        static void beginRenderingCmd(VkCommandBuffer commandBuffer,
                                      VkImage image,
                                      VkRenderingFlags renderingFlags,
                                      VkRect2D rectRenderSize,
                                      uint32_t layerCount,
                                      uint32_t viewMask,
                                      std::vector<AttachmentDescription> colorAttachmentDescList,
                                      const AttachmentDescription* depthAttachmentDescList,
                                      const AttachmentDescription* stencilAttachmentDescList,
                                      VkImageLayout oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                      VkImageLayout newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

        /// @brief 记录 `vkCmdEndRendering`，然后可选转换一张颜色图像的布局。
        /// @param[in] commandBuffer 当前处于 dynamic rendering 内的命令缓冲区。
        /// @param[in] image 在 @p oldLayout 与 @p newLayout 不同时执行布局转换的颜色图像。
        /// @param[in] oldLayout @p image 在可选 barrier 之前的布局。
        /// @param[in] newLayout @p image 在可选 barrier 之后的布局。
        /// @pre @p commandBuffer 必须处于由 `vkCmdBeginRendering` 开始且尚未结束的 rendering 内。
        /// @pre 当 @p oldLayout 与 @p newLayout 不同时，@p image 必须有效并支持指定的颜色图像布局转换。
        /// @note 当布局不同时，当前实现会在 `vkCmdEndRendering` 后记录一个从
        ///       `VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT` 到 `VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT` 的 barrier。
        /// @warning 此函数只处理 @p image；其他 attachment 和子资源的布局及同步仍由调用方负责。
        static void endRenderingCmd(VkCommandBuffer commandBuffer,
                                    VkImage image,
                                    VkImageLayout oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VkImageLayout newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    };

}  // namespace Huli::Vulkan
