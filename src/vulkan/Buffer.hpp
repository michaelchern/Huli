#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Common.hpp"
#include "Utility.hpp"
#include "vk_mem_alloc.h"

/// @file Buffer.hpp
/// @brief 声明 Vulkan 缓冲区及其 VMA 内存分配的 RAII 封装。

namespace Huli::Vulkan
{
    class Context;
    class Texture;

    /// @brief 管理 `VkBuffer`、VMA 内存分配和派生的缓冲区视图。
    ///
    /// `Buffer` 既可表示独立缓冲区，也可表示与目标 GPU 缓冲区关联的暂存缓冲区。
    /// 本对象拥有 `VkBuffer`、`VmaAllocation` 以及通过 requestBufferView() 创建的
    /// `VkBufferView`，并在析构时统一释放这些资源。
    ///
    /// @note `Context` 和 `VmaAllocator` 均不由本对象拥有，且必须比本对象存活更久。
    /// @note 暂存模式下的目标缓冲区同样不由本对象拥有；调用方必须保证它至少存活到
    ///       所有引用它的拷贝命令执行完成。
    /// @note 本类包含延迟映射和缓存写入，没有内部同步，不保证并发线程安全。
    /// @warning 当前默认移动操作不能安全转移并清空原始 Vulkan/VMA 句柄；在实现
    ///          自定义移动操作前，不要移动 `Buffer` 对象。
    class Buffer final
    {
    public:
        MOVABLE_ONLY(Buffer);

        /// @brief 创建与目标 GPU 缓冲区关联的暂存缓冲区。
        ///
        /// 构造函数会为 @p usage 补充 `VK_BUFFER_USAGE_TRANSFER_SRC_BIT`，并创建
        /// CPU 可访问的 VMA 内存分配。
        ///
        /// @param[in] context 上下文的非拥有指针，不能为空且必须比本对象存活更久。
        /// @param[in] vmaAllocator 有效的 VMA 分配器句柄，不转移所有权。
        /// @param[in] size 暂存缓冲区的大小，单位为字节。
        /// @param[in] usage 暂存缓冲区需要的额外用途标志。
        /// @param[in] actualBuffer 接收数据的非拥有目标缓冲区指针，不能为空。
        /// @param[in] name 可选的调试名称。
        /// @pre @p vmaAllocator 必须有效，并与 @p context 使用的 Vulkan 设备匹配。
        /// @pre @p actualBuffer 必须包含 `VK_BUFFER_USAGE_TRANSFER_DST_BIT`，并使用
        ///      `VMA_MEMORY_USAGE_GPU_ONLY` 分配内存。
        explicit Buffer(const Context* context,
                        VmaAllocator vmaAllocator,
                        VkDeviceSize size,
                        VkBufferUsageFlags usage,
                        Buffer* actualBuffer,
                        const std::string& name = "");

        /// @brief 根据 Vulkan 和 VMA 创建信息直接创建缓冲区。
        ///
        /// 此重载不关联目标缓冲区；缓冲区的用途和内存类型完全由传入的创建信息决定。
        ///
        /// @param[in] context 上下文的非拥有指针，不能为空且必须比本对象存活更久。
        /// @param[in] vmaAllocator 有效的 VMA 分配器句柄，不转移所有权。
        /// @param[in] createInfo Vulkan 缓冲区创建信息。
        /// @param[in] allocInfo VMA 内存分配信息。
        /// @param[in] name 可选的调试名称。
        /// @pre @p vmaAllocator、@p createInfo 和 @p allocInfo 必须满足 Vulkan/VMA
        ///      的有效性要求，且分配器必须与 @p context 使用的设备匹配。
        explicit Buffer(const Context* context,
                        VmaAllocator vmaAllocator,
                        const VkBufferCreateInfo& createInfo,
                        const VmaAllocationCreateInfo& allocInfo,
                        const std::string& name = "");

        /// @brief 解除映射，并释放缓冲区视图、缓冲区及其 VMA 内存分配。
        /// @note 析构函数不会等待 GPU；调用方必须先保证相关 GPU 操作已经结束。
        ~Buffer();

        /// @brief 获取缓冲区的大小。
        /// @return 缓冲区大小，单位为字节。
        VkDeviceSize size() const;

        /// @brief 获取底层 Vulkan 缓冲区句柄。
        /// @return 由本对象持有的 `VkBuffer`；调用方不得销毁该句柄。
        VkBuffer vkBuffer() const { return buffer_; }

        /// @brief 获取缓冲区设备地址。
        ///
        /// 暂存缓冲区返回其目标缓冲区的设备地址。其他缓冲区仅在 `_WIN32` 且定义
        /// `VK_KHR_buffer_device_address` 的构建中查询并缓存自身地址，否则返回 `0`。
        ///
        /// @return 可供设备访问的地址，或表示不可用的 `0`。
        /// @pre 调用方必须保证所需 feature 和缓冲区 usage 已启用；函数不会运行时检查。
        VkDeviceAddress vkDeviceAddress() const;

        /// @brief 刷新映射内存，使 CPU 写入对设备可见。
        /// @param[in] offset 刷新范围的起始偏移，单位为字节。
        /// @note 此函数调用 VMA 刷新操作，不会记录或提交 GPU 拷贝命令。
        /// @warning 当前重载始终使用 size() 作为刷新长度，因此实际范围从 @p offset
        ///          开始、长度为 size()；当 @p offset 非零时，该范围可能越界。
        void upload(VkDeviceSize offset = 0) const;

        /// @brief 刷新映射内存中的指定字节范围。
        /// @param[in] offset 刷新范围的起始偏移，单位为字节。
        /// @param[in] size 刷新范围的长度，单位为字节。
        /// @pre 刷新范围必须位于当前 VMA 内存分配之内。
        /// @note 此函数调用 VMA 刷新操作，不会记录或提交 GPU 拷贝命令。
        void upload(VkDeviceSize offset, VkDeviceSize size) const;

        /// @brief 将暂存缓冲区到目标缓冲区的拷贝命令记录到命令缓冲区。
        /// @param[in] commandBuffer 用于记录 `vkCmdCopyBuffer` 的命令缓冲区。
        /// @param[in] srcOffset 暂存缓冲区的起始偏移，单位为字节。
        /// @param[in] dstOffset 目标缓冲区的起始偏移，单位为字节。
        /// @pre 当前对象必须通过关联目标缓冲区的暂存构造函数创建。
        /// @pre @p commandBuffer 必须处于录制状态并支持传输命令。
        /// @pre 当前实现要求 @p srcOffset 为 `0`，且目标缓冲区必须能容纳从
        ///      @p dstOffset 开始的 size() 字节。
        /// @note 此函数只记录命令，不负责刷新内存、插入 barrier、提交命令缓冲区、
        ///       同步或等待执行完成。
        /// @warning 当前实现始终复制 size() 字节；调用方必须保证源和目标范围有效。
        void uploadStagingBufferToGPU(const VkCommandBuffer& commandBuffer,
                                      uint64_t srcOffset = 0,
                                      uint64_t dstOffset = 0) const;

        /// @brief 将 CPU 数据复制到缓冲区映射内存的起始位置。
        /// @param[in] data 源数据指针，不能为空。
        /// @param[in] size 要复制的数据长度，单位为字节。
        /// @pre 当前 VMA 内存分配必须允许 CPU 映射，且 @p size 不得超过缓冲区大小。
        /// @note 首次调用时会映射内存；此函数不会自动刷新非一致性内存。
        void copyDataToBuffer(const void* data, size_t size) const;

        /// @brief 获取指定格式对应的整段缓冲区视图。
        ///
        /// 每种格式只创建一个 `VkBufferView`，后续请求会返回缓存的句柄。
        ///
        /// @param[in] viewFormat 缓冲区视图使用的元素格式。
        /// @return 由本对象拥有的 `VkBufferView`；调用方不得销毁该句柄。
        /// @pre 缓冲区必须具有适用的 texel-buffer 用途，且格式、范围和对齐必须满足
        ///      Vulkan 与设备能力要求。
        VkBufferView requestBufferView(VkFormat viewFormat);

    private:
        /// 非拥有指针；`Context` 必须比本对象存活更久。
        const Context* context_ = nullptr;

        /// 非拥有句柄；分配器必须比本对象存活更久。
        VmaAllocator allocator_;

        VkDeviceSize size_;
        VkBufferUsageFlags usage_;
        VmaAllocationCreateInfo allocCreateInfo_;

        VkBuffer buffer_ = VK_NULL_HANDLE;

        /// 暂存模式下的非拥有目标缓冲区指针；为空表示当前对象不关联目标缓冲区。
        Buffer* actualBufferIfStaging_ = nullptr;

        VmaAllocation allocation_ = nullptr;
        VmaAllocationInfo allocationInfo_ = {};

        /// 首次查询后缓存的设备地址。
        mutable VkDeviceAddress bufferDeviceAddress_ = 0;

        /// 首次写入时获得的映射地址，由析构函数负责解除映射。
        mutable void* mappedMemory_ = nullptr;

        /// 按格式缓存且由本对象销毁的缓冲区视图。
        std::unordered_map<VkFormat, VkBufferView> bufferViews_;
    };

}  // namespace Huli::Vulkan
