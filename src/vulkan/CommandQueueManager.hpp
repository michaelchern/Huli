#pragma once

#include <functional>
#include <string>
#include <vector>

#include "Buffer.hpp"
#include "Common.hpp"
#include "Utility.hpp"

/// @file CommandQueueManager.hpp
/// @brief 声明 command pool、command buffer、提交 fence 与延迟释放资源的管理器。

namespace Huli::Vulkan
{

    class Context;

    /// @brief 管理一个 Vulkan queue 对应的 command pool、command buffer 环和提交 fence 环。
    ///
    /// 本对象拥有内部创建的 `VkCommandPool`、预分配的 `VkCommandBuffer` 和 `VkFence`，并在析构时释放。
    /// `VkDevice` 与 `VkQueue` 仅作为非拥有句柄保存；构造时传入的 `Context` 只用于设置 debug object name。
    ///
    /// @note 本类没有内部同步；对 queue 的提交、索引推进和延迟资源容器的访问均应由调用方串行化。
    /// @warning 析构函数不会等待 GPU。销毁前必须保证所有提交均已完成，并且 device 与 queue 仍然有效。
    /// @warning 当前类没有显式禁止 copy，也没有可安全转移所有权的 move 操作。复制或移动对象会造成 Vulkan
    ///          句柄的重复所有权；只能直接构造或依赖 C++17 的保证复制消除接收工厂返回值。
    /// @warning 当前延迟 deallocator 的清理会遍历所有 fence slot，且执行后不会从容器移除，可能提前或重复调用。
    class CommandQueueManager final
    {
    public:
        /// @brief 创建 command pool，并预分配 command buffer 与提交 fence。
        /// @param[in] context 仅在构造期间用于设置 Vulkan debug object name 的上下文。
        /// @param[in] device 创建和销毁内部资源所用的非拥有逻辑设备句柄。
        /// @param[in] count command buffer 环中的预分配数量。
        /// @param[in] concurrentNumCommands fence 环和延迟资源槽的数量，即允许同时在途的提交数量。
        /// @param[in] queueFamilyIndex @p queue 所属的 queue family 下标。
        /// @param[in] queue 接收提交的非拥有 queue 句柄。
        /// @param[in] flags command pool 创建标志；默认允许单独重置 command buffer。
        /// @param[in] name command pool 和预分配 command buffer 的可选 debug name 后缀。
        /// @pre @p device、@p queue 与 @p queueFamilyIndex 必须有效且彼此匹配。
        /// @pre @p count 和 @p concurrentNumCommands 必须大于 `0`，且 @p count 不得小于
        ///      @p concurrentNumCommands，以免重用仍在 GPU 上执行的 command buffer。
        /// @pre 若使用 getCmdBufferToBegin()，@p flags 必须包含 `VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT`。
        explicit CommandQueueManager(const Context& context,
                                     VkDevice device,
                                     uint32_t count,
                                     uint32_t concurrentNumCommands,
                                     uint32_t queueFamilyIndex,
                                     VkQueue queue,
                                     VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                     const std::string& name = "");

        /// @brief 执行已登记的 deallocator，并销毁 fence、预分配 command buffer 和 command pool。
        /// @pre 所有使用内部资源的 GPU 操作必须已经完成，且 device_ 保存的句柄必须仍然有效。
        /// @warning 此函数不会等待 queue 或 device idle，并且会再次执行仍保存在 deallocators_ 中的回调。
        ~CommandQueueManager();

        /// @brief 使用当前 fence slot 向受管 queue 提交一次工作。
        /// @param[in] submitInfo 有效的 Vulkan 提交信息；只在调用期间读取，不接管其内存。
        /// @pre 当前 fence 不得仍被未完成的提交使用，且提交信息中的 command buffer 必须已经结束录制。
        /// @note 此函数会重置当前 fence 并标记该 slot 已提交，但不会结束 command buffer、推进索引或等待完成。
        void submit(const VkSubmitInfo* submitInfo);

        /// @brief 将 command buffer 和 fence 的当前索引分别推进到各自环中的下一项。
        /// @pre 两个环都必须非空；通常应在当前 slot 的提交和延迟资源登记完成后调用。
        /// @note 此函数只修改索引，不提交、不等待，也不释放延迟资源。
        void goToNextCmdBuffer();

        /// @brief 等待当前 fence slot 对应的提交完成并释放该 slot 保留的 Buffer。
        ///
        /// 未标记为已提交的 slot 会直接返回。等待超时时，当前实现输出错误并调用 `vkDeviceWaitIdle()`。
        ///
        /// @note 当前 fence 会保留 signaled 状态，下一次 submit() 会在提交前将其重置。
        /// @warning 随后的 deallocator 清理会执行所有 slot 中的全部回调，且不会清空回调容器。
        void waitUntilSubmitIsComplete();

        /// @brief 依次等待所有 fence，释放全部延迟 Buffer，并执行已登记的 deallocator。
        /// @warning 当前实现会重置所有 fence、清空 bufferToDispose_ 的外层容器，并保留已执行的回调；调用后不应再
        ///          复用本管理器录制、提交或登记资源，应将其视为析构前的终止性清理。
        void waitUntilAllSubmitsAreComplete();

        /// @brief 将 Buffer 的共享所有权绑定到当前 fence slot，延迟到相关提交完成后释放。
        /// @param[in] buffer 需要延长生命周期的 Buffer；共享所有权被移动到管理器。
        /// @pre 必须在推进当前 slot 之前调用，并确保相关 GPU 命令通过当前 fence 提交。
        /// @note 此函数只登记资源，不等待提交完成。
        void disposeWhenSubmitCompletes(std::shared_ptr<Buffer> buffer);

        /// @brief 将资源释放回调登记到当前 fence slot。
        /// @param[in] deallocator 有效且不应抛出异常的释放回调；其所有权被移动到管理器。
        /// @pre 必须在推进当前 slot 之前调用，并确保相关 GPU 命令通过当前 fence 提交。
        /// @warning 当前实现清理任意 slot 时都会遍历所有回调，且执行后不移除回调，因此可能提前并重复调用。
        void disposeWhenSubmitCompletes(std::function<void()>&& deallocator);

        /// @brief 等待当前 fence，重置并开始录制当前预分配 command buffer。
        /// @return 由本对象拥有、已处于 recording 状态的 primary command buffer。
        /// @pre command pool 必须允许单独重置 command buffer。
        /// @note command buffer 使用 `VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT` 开始录制。
        /// @note 此函数等待 fence 后不会更新 isSubmitted_，也不会释放延迟资源；需要时应先调用
        ///       waitUntilSubmitIsComplete()。
        VkCommandBuffer getCmdBufferToBegin();

        /// @brief 从内部 command pool 额外分配一个 primary command buffer。
        /// @return 尚未开始录制且不属于预分配环的 command buffer。
        /// @note 返回句柄不会加入 commandBuffers_，但仍从内部 pool 分配，并会在 command pool 销毁时失效。
        VkCommandBuffer getCmdBuffer();

        /// @brief 结束指定 command buffer 的录制。
        /// @param[in] cmdBuffer 当前处于 recording 状态的有效 command buffer。
        /// @note 此函数不会提交 command buffer。
        void endCmdBuffer(VkCommandBuffer cmdBuffer);

        /// @brief 获取受管 queue 所属的 queue family 下标。
        uint32_t queueFamilyIndex() const { return queueFamilyIndex_; }
    private:
        /// @brief 执行 deallocators_ 中所有 slot 保存的回调。
        /// @warning 当前实现不会检查对应 fence，也不会在执行后清空回调。
        void deallocateResources();
    private:
        /// fence 环、提交状态和延迟资源容器的 slot 数量。
        uint32_t commandsInFlight_ = 2;

        uint32_t queueFamilyIndex_ = 0;

        /// 非拥有 queue 句柄，必须比本对象存活更久。
        VkQueue queue_ = VK_NULL_HANDLE;

        /// 非拥有 device 句柄，必须比本对象存活更久。
        VkDevice device_ = VK_NULL_HANDLE;

        /// 由本对象创建并销毁的 command pool。
        VkCommandPool commandPool_ = VK_NULL_HANDLE;

        /// 构造时预分配、由本对象显式释放的 command buffer 环。
        std::vector<VkCommandBuffer> commandBuffers_;

        /// 与在途提交 slot 一一对应、由本对象销毁的 fence 环。
        std::vector<VkFence> fences_;

        /// 记录各 fence slot 是否已通过 submit() 提交。
        std::vector<bool> isSubmitted_;

        uint32_t fenceCurrentIndex_ = 0;
        uint32_t commandBufferCurrentIndex_ = 0;

        /// 按 fence slot 保留 Buffer 的共享所有权，防止 GPU 使用期间提前析构。
        std::vector<std::vector<std::shared_ptr<Buffer>>> bufferToDispose_;

        /// 按 fence slot 登记的资源释放回调；当前清理实现不会移除已经执行的回调。
        std::vector<std::vector<std::function<void()>>> deallocators_;
    };

}  // namespace Huli::Vulkan
