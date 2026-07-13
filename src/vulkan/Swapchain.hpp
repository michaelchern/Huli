#pragma once

#include <memory>
#include <vector>

#include "Common.hpp"
#include "Utility.hpp"

/// @file Swapchain.hpp
/// @brief 声明 Vulkan 交换链、交换链图像及呈现同步资源的 RAII 封装。

namespace Huli::Vulkan
{

    class Context;
    class Framebuffer;
    class PhysicalDevice;
    class Texture;

    /// @brief 管理 `VkSwapchainKHR`、交换链图像包装对象以及获取和呈现所需的同步资源。
    ///
    /// 构造函数会创建交换链，从交换链取得 `VkImage`，并使用 `Texture` 为每张图像创建
    /// 图像视图。交换链图像本身仍由 `VkSwapchainKHR` 拥有，`Texture` 只负责包装图像并
    /// 管理对应的图像视图。本对象还拥有图像可用信号量、渲染完成信号量和图像获取 fence。
    ///
    /// @note `VkDevice` 和呈现队列只以句柄形式借用；其底层对象必须比本对象存活更久。
    /// @note 本类只提供单组二进制信号量和一个 acquire fence，调用方必须按顺序完成
    ///       acquire、提交和 present，且不能并发复用这些同步对象。
    /// @warning 当前默认构造函数不会创建任何 Vulkan 资源，而析构函数没有空句柄保护；
    ///          不要单独创建并销毁默认构造的 `Swapchain`。
    /// @warning 当前类没有禁用复制操作；复制会重复持有同一组 Vulkan 句柄并导致重复销毁，
    ///          因此不要复制 `Swapchain` 对象。
    class Swapchain final
    {
    public:
        /// @brief 创建一个未初始化的交换链占位对象。
        /// @warning 该对象不能用于任何交换链操作，也不能在未完成有效初始化时直接析构。
        explicit Swapchain() = default;

        /// @brief 创建交换链、交换链图像包装对象以及 acquire/present 同步资源。
        /// @param[in] context 提供逻辑设备和 Vulkan 对象命名能力的上下文。
        /// @param[in] physicalDevice 提供表面能力和图形、呈现队列族索引的物理设备信息。
        /// @param[in] surface 用于创建交换链的有效表面句柄；所有权不会转移。
        /// @param[in] presentQueue 支持向 @p surface 呈现的队列；所有权不会转移。
        /// @param[in] imageFormat 交换链图像格式。
        /// @param[in] imageClorSpace 交换链图像色彩空间。
        /// @param[in] presentMode 交换链呈现模式。
        /// @param[in] extent 交换链图像的宽度和高度。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre @p context、@p physicalDevice、@p surface 和 @p presentQueue 必须属于同一
        ///      Vulkan 设备与表面配置，且图形和呈现队列族索引必须有效。
        /// @pre 图像格式、色彩空间、呈现模式和范围必须受目标表面支持。
        /// @warning 当前实现直接使用 `surfaceCapabilities().maxImageCount` 作为 `std::clamp`
        ///          上限；当 Vulkan 以 `0` 表示没有上限时，上下限可能不满足其前置条件。
        explicit Swapchain(const Context& context,
                           const PhysicalDevice& physicalDevice,
                           VkSurfaceKHR surface,
                           VkQueue presentQueue,
                           VkFormat imageFormat,
                           VkColorSpaceKHR imageClorSpace,
                           VkPresentModeKHR presentMode,
                           VkExtent2D extent,
                           const std::string& name = "");

        /// @brief 等待当前图像获取 fence，并销毁 fence、信号量和交换链。
        /// @note 析构函数不会等待呈现队列或设备空闲；调用方必须先保证 GPU 不再使用相关资源。
        ~Swapchain();

        /// @brief 获取交换链图像数量。
        /// @return 当前缓存的交换链图像包装对象数量。
        uint32_t numberImages() const { return static_cast<uint32_t>(images_.size()); }

        /// @brief 获取最近一次成功获取的交换链图像索引。
        /// @return images_ 容器中的当前图像索引；首次 acquire 前该值为 `0`。
        size_t currentImageIndex() const { return imageIndex_; }

        /// @brief 等待并重置 acquire fence，然后阻塞获取下一张交换链图像。
        /// @return 当前交换链图像的共享包装对象。
        /// @note `vkAcquireNextImageKHR` 使用无限超时，并在图像可用时发出 imageAvailable_ 信号。
        /// @warning 当前实现通过 `VK_CHECK` 处理返回值，没有单独恢复
        ///          `VK_ERROR_OUT_OF_DATE_KHR` 或 `VK_SUBOPTIMAL_KHR`。
        std::shared_ptr<Texture> acquireImage();

        /// @brief 获取交换链图像格式。
        /// @return 创建交换链时使用的 `VkFormat`。
        VkFormat imageFormat() const { return imageFormat_; }

        /// @brief 获取交换链图像范围。
        /// @return 创建交换链时使用的宽度和高度。
        VkExtent2D extent() const { return extent_; }

        /// @brief 等待渲染完成信号量，并将当前图像提交到呈现队列。
        /// @pre 当前图像必须已成功获取，且先前的队列提交最终会发出 imageRendered_ 信号。
        /// @note 此函数直接调用 `vkQueuePresentKHR`，不会重建失效或次优的交换链。
        void present() const;

        /// @brief 生成一次队列提交使用的 `VkSubmitInfo`。
        /// @param[in] buffer 要提交的命令缓冲区句柄存储地址，不能为空。
        /// @param[in] submitStageMask 等待 imageAvailable_ 时使用的目标 pipeline stage 地址。
        /// @param[in] waitForImageAvailable 是否等待图像可用信号量。
        /// @param[in] signalImagePresented 是否在命令执行完成后发出渲染完成信号量。
        /// @return 引用传入命令缓冲区、stage mask 和本对象信号量的提交信息。
        /// @pre 返回的 `VkSubmitInfo` 被队列提交消费前，@p buffer 和本对象必须保持有效；
        ///      启用 @p waitForImageAvailable 时，@p submitStageMask 也必须非空且保持有效。
        /// @note 此函数只组装结构体，不会调用 `vkQueueSubmit`。
        VkSubmitInfo createSubmitInfo(const VkCommandBuffer* buffer,
                                      const VkPipelineStageFlags* submitStageMask,
                                      bool waitForImageAvailable = true,
                                      bool signalImagePresented = true) const;

        /// @brief 获取指定索引的交换链图像包装对象。
        /// @param[in] index images_ 中的图像索引。
        /// @return 对应 `Texture` 的共享所有权指针。
        /// @pre @p index 必须小于 numberImages()。
        std::shared_ptr<Texture> texture(uint32_t index) const
        {
            ASSERT(index < images_.size(),
                   "Index is greater than number of images in the swapchain");
            return images_[index];
        }
    private:
        /// @brief 查询交换链图像，并为每张图像创建非拥有的 `Texture` 包装对象。
        void createTextures(const Context& context, VkFormat imageFormat, const VkExtent2D& extent);

        /// @brief 创建图像可用和渲染完成二进制信号量。
        void createSemaphores(const Context& context);
    private:
        /// 创建交换链时借用的逻辑设备句柄。
        VkDevice device_ = VK_NULL_HANDLE;

        /// 由本对象创建并销毁的交换链句柄。
        VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;

        /// 创建交换链时借用的呈现队列句柄。
        VkQueue presentQueue_ = VK_NULL_HANDLE;

        /// 交换链图像的包装对象；包装对象拥有图像视图，但不拥有底层交换链图像。
        std::vector<std::shared_ptr<Texture>> images_;

        /// acquire 成功后由 Vulkan 发出信号，供渲染提交等待。
        VkSemaphore imageAvailable_ = VK_NULL_HANDLE;

        /// 渲染提交完成后发出信号，供 present 等待。
        VkSemaphore imageRendered_ = VK_NULL_HANDLE;

        /// 最近一次 acquire 返回的交换链图像索引。
        uint32_t imageIndex_ = 0;

        /// 创建交换链时记录的图像范围。
        VkExtent2D extent_;

        /// 创建交换链时记录的图像格式。
        VkFormat imageFormat_;

        /// 用于串行化图像获取操作的 fence。
        VkFence acquireFence_ = VK_NULL_HANDLE;
    };

}  // namespace Huli::Vulkan
