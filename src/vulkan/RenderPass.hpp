#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Common.hpp"
#include "Utility.hpp"

/// @file RenderPass.hpp
/// @brief 声明 Vulkan render pass 的 RAII 封装。

namespace Huli::Vulkan
{

    class Context;
    class Texture;

    /// @brief 管理单个 Vulkan render pass 句柄。
    ///
    /// 当前实现使用 `vkCreateRenderPass` 创建一个 subpass，并在构造时建立固定的外部依赖。
    /// 构造参数中的 attachment 描述只在创建期间读取，不会由本对象长期持有。
    ///
    /// @note 本对象拥有创建得到的 `VkRenderPass`，并在析构时将其销毁。
    /// @note 本对象只保存非拥有的 `VkDevice` 句柄；对应逻辑设备必须比本对象存活更久。
    /// @warning 当前默认移动操作只会复制 Vulkan 句柄，不会清空源对象，可能导致重复销毁；
    ///          在实现安全的自定义移动操作前，不要移动 `RenderPass` 对象。
    class RenderPass final
    {
    public:
        MOVABLE_ONLY(RenderPass);

        /// @brief 根据 `Texture` 的格式、采样数和当前布局创建 render pass。
        /// @param[in] context 提供逻辑设备和调试命名能力的上下文；其逻辑设备必须比本对象存活更久。
        /// @param[in] attachments 主 attachment 列表；顺序决定 attachment index，纹理对象不会被本对象保存。
        /// @param[in] resolveAttachments resolve attachment 列表；创建信息会将其追加到主 attachment 之后。
        /// @param[in] loadOp 每个主 attachment 及 resolve attachment 的加载操作。
        /// @param[in] storeOp 每个主 attachment 及 resolve attachment 的存储操作。
        /// @param[in] layout 每个主 attachment 及 resolve attachment 的最终布局。
        /// @param[in] bindPoint 预期的 pipeline bind point。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre @p loadOp、@p storeOp 和 @p layout 的元素数量必须等于两个 attachment 列表的元素总数。
        /// @pre @p resolveAttachments 必须为空，或与 color attachment 数量匹配；所有输入必须满足 Vulkan
        ///      render pass 与 resolve attachment 的兼容性要求。
        /// @pre 主 attachment 中最多只能形成一个 depth/stencil attachment reference。
        /// @warning 当前实现未执行上述数量检查，并固定使用 `VK_PIPELINE_BIND_POINT_GRAPHICS`，不会读取 @p bindPoint。
        RenderPass(const Context& context,
                   const std::vector<std::shared_ptr<Texture>> attachments,
                   const std::vector<std::shared_ptr<Texture>> resolveAttachments,
                   const std::vector<VkAttachmentLoadOp>& loadOp,
                   const std::vector<VkAttachmentStoreOp>& storeOp,
                   const std::vector<VkImageLayout>& layout,
                   VkPipelineBindPoint bindPoint,
                   const std::string& name = "");

        /// @brief 根据显式 attachment 描述创建单采样 render pass。
        /// @param[in] context 提供逻辑设备和调试命名能力的上下文；其逻辑设备必须比本对象存活更久。
        /// @param[in] formats 按 attachment index 排列的格式列表；所有 attachment 的采样数固定为 `1`。
        /// @param[in] initialLayouts 每个 attachment 的初始布局。
        /// @param[in] finalLayouts 每个 attachment 的最终布局。
        /// @param[in] loadOp 每个 attachment 的加载操作。
        /// @param[in] storeOp 每个 attachment 的存储操作。
        /// @param[in] bindPoint 预期的 pipeline bind point。
        /// @param[in] resolveAttachmentsIndices 预期作为 resolve attachment 的索引列表。
        /// @param[in] depthAttachmentIndex depth attachment 索引；`UINT32_MAX` 表示不指定。
        /// @param[in] stencilAttachmentIndex stencil attachment 索引；`UINT32_MAX` 表示不指定。
        /// @param[in] stencilLoadOp stencil 分量的加载操作。
        /// @param[in] stencilStoreOp stencil 分量的存储操作。
        /// @param[in] multiview 是否接入固定 view mask `0x00000003`，即启用 view `0` 和 `1`。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre 五个 attachment 属性列表的元素数量必须相同。
        /// @pre 所有非 `UINT32_MAX` 的 attachment index 必须位于 @p formats 范围内；如果同时指定 depth 和
        ///      stencil，它们应引用同一个 depth/stencil attachment。
        /// @warning 当前实现固定使用 `VK_PIPELINE_BIND_POINT_GRAPHICS`，不会读取 @p bindPoint。
        /// @warning 当前实现不会读取 @p resolveAttachmentsIndices，因此不会为此重载建立 resolve attachment reference。
        RenderPass(const Context& context,
                   const std::vector<VkFormat>& formats,
                   const std::vector<VkImageLayout>& initialLayouts,
                   const std::vector<VkImageLayout>& finalLayouts,
                   const std::vector<VkAttachmentLoadOp>& loadOp,
                   const std::vector<VkAttachmentStoreOp>& storeOp,
                   VkPipelineBindPoint bindPoint,
                   std::vector<uint32_t> resolveAttachmentsIndices,
                   uint32_t depthAttachmentIndex,
                   uint32_t stencilAttachmentIndex = UINT32_MAX,
                   VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                   VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                   bool multiview = false,
                   const std::string& name = "");

        /// @brief 创建可附带 fragment density map 信息的单采样 render pass。
        /// @param[in] context 提供逻辑设备和调试命名能力的上下文；其逻辑设备必须比本对象存活更久。
        /// @param[in] formats 按 attachment index 排列的格式列表；所有 attachment 的采样数固定为 `1`。
        /// @param[in] initialLayouts 每个 attachment 的初始布局。
        /// @param[in] finalLayouts 每个 attachment 的最终布局。
        /// @param[in] loadOp 每个 attachment 的加载操作。
        /// @param[in] storeOp 每个 attachment 的存储操作。
        /// @param[in] bindPoint 预期的 pipeline bind point。
        /// @param[in] resolveAttachmentsIndices 预期作为 resolve attachment 的索引列表。
        /// @param[in] depthAttachmentIndex depth attachment 索引；`UINT32_MAX` 表示不指定。
        /// @param[in] fragmentDensityMapIndex fragment density map attachment 索引；`UINT32_MAX` 表示不指定。
        /// @param[in] stencilAttachmentIndex stencil attachment 索引；`UINT32_MAX` 表示不指定。
        /// @param[in] stencilLoadOp stencil 分量的加载操作。
        /// @param[in] stencilStoreOp stencil 分量的存储操作。
        /// @param[in] multiview 是否接入固定 view mask `0x00000003`，即启用 view `0` 和 `1`。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre 五个 attachment 属性列表的元素数量必须相同。
        /// @pre 所有非 `UINT32_MAX` 的 attachment index 必须位于 @p formats 范围内；如果同时指定 depth 和
        ///      stencil，它们应引用同一个 depth/stencil attachment。
        /// @pre 使用 fragment density map 时，逻辑设备必须已启用 `VK_EXT_fragment_density_map`，并将
        ///      @p fragmentDensityMapIndex 指向兼容的 attachment。
        /// @warning 当前实现固定使用 `VK_PIPELINE_BIND_POINT_GRAPHICS`，不会读取 @p bindPoint。
        /// @warning 当前实现不会读取 @p resolveAttachmentsIndices，因此不会建立 resolve attachment reference。
        /// @warning fragment density map 创建信息仅在定义 `VK_EXT_fragment_density_map` 且 @p multiview 为 `true`
        ///          时接入 `pNext` 链；此重载仍然调用 `vkCreateRenderPass`。
        RenderPass(const Context& context,
                   const std::vector<VkFormat>& formats,
                   const std::vector<VkImageLayout>& initialLayouts,
                   const std::vector<VkImageLayout>& finalLayouts,
                   const std::vector<VkAttachmentLoadOp>& loadOp,
                   const std::vector<VkAttachmentStoreOp>& storeOp,
                   VkPipelineBindPoint bindPoint,
                   std::vector<uint32_t> resolveAttachmentsIndices,
                   uint32_t depthAttachmentIndex,
                   uint32_t fragmentDensityMapIndex,
                   uint32_t stencilAttachmentIndex = UINT32_MAX,
                   VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                   VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                   bool multiview = false,
                   const std::string& name = "");

        /// @brief 销毁本对象拥有的 Vulkan render pass。
        /// @pre 对应逻辑设备必须仍然有效，且所有引用该 render pass 的 GPU 操作必须已经结束。
        ~RenderPass();

        /// @brief 获取底层 Vulkan render pass 句柄。
        /// @return 本对象拥有的 `VkRenderPass`；调用方不得销毁该句柄。
        VkRenderPass vkRenderPass() const;
    private:
        /// 非拥有句柄；对应逻辑设备必须比本对象存活更久。
        VkDevice device_ = VK_NULL_HANDLE;

        /// 由本对象创建并销毁的 Vulkan render pass。
        VkRenderPass renderPass_ = VK_NULL_HANDLE;
    };

}  // namespace Huli::Vulkan
