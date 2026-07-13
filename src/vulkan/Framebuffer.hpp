#pragma once

#include <memory>
#include <vector>

#include "Common.hpp"
#include "Utility.hpp"

/// @file Framebuffer.hpp
/// @brief 声明 Vulkan framebuffer 的 RAII 封装。

namespace Huli::Vulkan
{
    class Context;
    class Texture;

    /// @brief 管理一个由固定 attachment 图像视图创建的 `VkFramebuffer`。
    ///
    /// `Framebuffer` 只拥有底层 `VkFramebuffer`。构造时传入的 `VkDevice`、`VkRenderPass`
    /// 和各个 `Texture` 均不由本对象拥有；虽然 attachment 使用 `std::shared_ptr` 传入，
    /// 构造函数也不会保存这些智能指针。
    ///
    /// @note 调用方必须保证逻辑设备和所有 attachment 图像视图至少存活到本对象析构，
    ///       并保证 render pass 与 attachment 的格式及排列顺序兼容。
    /// @note 本类不提供内部同步，不保证并发线程安全。
    /// @warning 当前默认移动操作只会复制 Vulkan 句柄，无法清空源对象；在实现安全的
    ///          自定义移动操作前，不要移动 `Framebuffer` 对象。
    class Framebuffer final
    {
    public:
        MOVABLE_ONLY(Framebuffer);

        /// @brief 根据颜色、深度和模板 attachment 创建 framebuffer。
        ///
        /// attachment 图像视图按照颜色列表、深度、模板的顺序写入创建信息；所有图像视图
        /// 均取自 mip `0`，framebuffer 的层数固定为 `1`。
        ///
        /// @param[in] context 用于设置 Vulkan 调试名称的上下文，仅在构造期间使用。
        /// @param[in] device 创建并销毁 framebuffer 的逻辑设备句柄，不转移所有权。
        /// @param[in] renderPass 与 attachment 格式和顺序兼容的 render pass，不转移所有权。
        /// @param[in] attachments 颜色 attachment 列表；顺序决定对应的 attachment index。
        /// @param[in] depthAttachment 可选的深度 attachment，追加在所有颜色 attachment 之后。
        /// @param[in] stencilAttachment 可选的模板 attachment，追加在深度 attachment 之后。
        /// @param[in] name 可选的调试名称。
        /// @pre @p device、@p renderPass 以及所有非空 attachment 必须有效，并属于兼容的 Vulkan 设备。
        /// @pre 颜色 attachment 至少包含一个元素，或者 @p depthAttachment 非空；当前实现不支持
        ///      只有模板 attachment 的情况。
        /// @pre attachment 中的 `Texture` 不得为空，且其格式、采样数、用途、尺寸和排列顺序必须
        ///      满足 @p renderPass 与 Vulkan framebuffer 的兼容性要求。
        /// @note framebuffer 的宽高取第一个颜色 attachment；没有颜色 attachment 时取深度 attachment。
        explicit Framebuffer(const Context& context,
                             VkDevice device,
                             VkRenderPass renderPass,
                             const std::vector<std::shared_ptr<Texture>>& attachments,
                             const std::shared_ptr<Texture> depthAttachment,
                             const std::shared_ptr<Texture> stencilAttachment,
                             const std::string& name = "");

        /// @brief 销毁本对象拥有的 Vulkan framebuffer。
        /// @note 析构函数不会等待 GPU；调用方必须先保证所有引用该 framebuffer 的 GPU 工作已经完成。
        ~Framebuffer();

        /// @brief 获取底层 Vulkan framebuffer 句柄。
        /// @return 由本对象拥有的 `VkFramebuffer`；调用方不得销毁该句柄。
        VkFramebuffer vkFramebuffer() const;
    private:
        /// 创建并销毁 framebuffer 所用的非拥有逻辑设备句柄。
        VkDevice device_ = VK_NULL_HANDLE;

        /// 由本对象创建并在析构时销毁的 framebuffer 句柄。
        VkFramebuffer framebuffer_ = VK_NULL_HANDLE;
    };

}  // namespace Huli::Vulkan
