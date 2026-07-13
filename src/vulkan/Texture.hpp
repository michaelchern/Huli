#pragma once

#include <unordered_map>

#include "Common.hpp"
#include "Utility.hpp"
#include "vk_mem_alloc.h"

/// @file Texture.hpp
/// @brief 声明 Vulkan 图像、图像视图及 mipmap 操作的 RAII 封装。

namespace Huli::Vulkan
{

    class Buffer;
    class Context;

    /// @brief 管理 `VkImage`、VMA 内存分配、图像视图和图像布局跟踪。
    ///
    /// `Texture` 可以创建并拥有一张 VMA 图像，也可以包装交换链等外部创建的图像。
    /// 两种模式都会为图像创建主 `VkImageView`，并由本对象负责销毁该视图。
    ///
    /// @note `Context` 由本对象以引用方式借用，必须比本对象存活更久。
    /// @note 布局成员只记录 CPU 侧认为的整张图像布局，不会自动查询 GPU 的实际状态。
    /// @note 本类没有内部同步，不保证并发线程安全。
    /// @warning 当前默认移动构造会复制 Vulkan/VMA 句柄而不会清空源对象；在实现
    ///          自定义移动操作前，不要移动 `Texture` 对象。
    class Texture final
    {
    public:
        MOVABLE_ONLY(Texture);

        /// @brief 创建并拥有一张由 VMA 分配内存的 Vulkan 图像。
        /// @param[in] context Vulkan 上下文，必须比本对象存活更久。
        /// @param[in] type 图像类型，例如一维、二维或三维图像。
        /// @param[in] format 图像像素格式。
        /// @param[in] flags Vulkan 图像创建标志。
        /// @param[in] usageFlags 图像用途标志。
        /// @param[in] extents 图像在各维度上的范围。
        /// @param[in] numMipLevels 请求的 mip 层数；启用 @p generateMips 时会重新计算。
        /// @param[in] layerCount 图像数组层数。
        /// @param[in] memoryFlags 用于选择 VMA 内存偏好的 Vulkan 内存属性。
        /// @param[in] generateMips 是否根据图像尺寸自动计算并生成 mipmap。
        /// @param[in] msaaSamples 图像的多重采样数。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @param[in] multiview 是否按多视图图像创建数组视图。
        /// @param[in] imageTiling 图像的 tiling 模式。
        /// @pre @p extents 的宽度和高度必须大于 `0`，@p numMipLevels 必须大于 `0`。
        /// @pre 多重采样图像只能包含一个 mip 层。
        /// @pre 若启用 @p generateMips，格式和 usage 必须支持线性 blit 以及传输读写。
        explicit Texture(const Context& context,
                         VkImageType type,
                         VkFormat format,
                         VkImageCreateFlags flags,
                         VkImageUsageFlags usageFlags,
                         VkExtent3D extents,
                         uint32_t numMipLevels,
                         uint32_t layerCount,
                         VkMemoryPropertyFlags memoryFlags,
                         bool generateMips = false,
                         VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT,
                         const std::string& name = "",
                         bool multiview = false,
                         VkImageTiling imageTiling = VK_IMAGE_TILING_OPTIMAL);

        /// @brief 包装一张由外部创建和拥有的 Vulkan 图像，例如交换链图像。
        /// @param[in] context Vulkan 上下文，必须比本对象存活更久。
        /// @param[in] device 与图像关联的逻辑设备。
        /// @param[in] image 外部拥有的有效图像句柄；所有权不会转移。
        /// @param[in] format 图像像素格式。
        /// @param[in] extents 图像范围。
        /// @param[in] numlayers 图像数组层数。
        /// @param[in] multiview 是否为图像创建二维数组视图。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @note 当前实现保留 @p device 参数以兼容接口，但创建视图时使用
        ///       `context.device()`。
        explicit Texture(const Context& context,
                         VkDevice device,
                         VkImage image,
                         VkFormat format,
                         VkExtent3D extents,
                         uint32_t numlayers = 1,
                         bool multiview = false,
                         const std::string& name = "");

        /// @brief 销毁本对象创建的全部图像视图，并在拥有图像时释放 VMA 图像。
        /// @note 析构函数不会等待 GPU；调用方必须先保证相关命令已经执行完成。
        ~Texture();

        /// @brief 获取图像像素格式。
        /// @return 创建或包装图像时记录的 `VkFormat`。
        VkFormat vkFormat() const { return format_; }

        /// @brief 获取主图像视图，或请求一个按 mip 层缓存的图像视图。
        /// @param[in] mipLevel 目标 mip 层；`UINT32_MAX` 表示返回主视图。
        /// @return 由本对象拥有的 `VkImageView`；调用方不得销毁该句柄。
        /// @pre @p mipLevel 必须为 `UINT32_MAX` 或小于 numMipLevels()。
        /// @warning 当前 `createImageView()` 固定从 mip `0` 开始，因此非默认请求虽然按
        ///          @p mipLevel 缓存，创建出的视图仍指向 mip `0`。
        VkImageView vkImageView(uint32_t mipLevel = UINT32_MAX);

        /// @brief 获取底层 Vulkan 图像句柄。
        /// @return 本对象创建或包装的 `VkImage`；调用方不得通过此接口转移所有权。
        VkImage vkImage() const { return image_; }

        /// @brief 获取图像范围。
        /// @return 创建或包装图像时记录的 `VkExtent3D`。
        VkExtent3D vkExtents() const { return extents_; }

        /// @brief 获取 CPU 侧记录的图像布局。
        /// @return 最近记录或显式设置的 `VkImageLayout`。
        VkImageLayout vkLayout() const { return layout_; }

        /// @brief 仅更新 CPU 侧记录的图像布局。
        /// @param[in] layout 要记录的图像布局。
        /// @warning 此函数不会记录 pipeline barrier，也不会改变 GPU 上的实际布局。
        void setImageLayout(VkImageLayout layout) { layout_ = layout; }

        /// @brief 获取 VMA 分配占用的设备内存大小。
        /// @return VMA 图像的分配大小；包装外部图像时通常为 `0`。
        VkDeviceSize vkDeviceSize() const { return deviceSize_; }

        /// @brief 上传基础 mip 数据、按需生成 mipmap，并转换为着色器只读布局。
        /// @param[in] cmdBuffer 用于记录传输、blit 和 barrier 命令的命令缓冲区。
        /// @param[in] stagingBuffer 保存上传数据的暂存缓冲区，不能为空。
        /// @param[in] data 要复制到暂存缓冲区的 CPU 数据，不能为空。
        /// @pre 命令缓冲区必须处于录制状态，暂存缓冲区容量必须足够。
        /// @note 此函数只记录命令，不提交命令缓冲区或等待 GPU 完成。
        void uploadAndGenMips(VkCommandBuffer cmdBuffer, const Buffer* stagingBuffer, void* data);

        /// @brief 将 CPU 数据上传到指定数组层的基础 mip。
        /// @param[in] cmdBuffer 用于记录布局转换和拷贝命令的命令缓冲区。
        /// @param[in] stagingBuffer 保存上传数据的暂存缓冲区，不能为空。
        /// @param[in] data 要上传的 CPU 数据，不能为空。
        /// @param[in] layer 目标图像数组层。
        /// @pre @p layer 必须小于图像数组层数，暂存缓冲区必须能够容纳基础 mip 数据。
        /// @note 此函数只记录命令；不会提交、等待，也不会将图像转为着色器只读布局。
        void uploadOnly(VkCommandBuffer cmdBuffer,
                        const Buffer* stagingBuffer,
                        void* data,
                        uint32_t layer = 0);

        /// @brief 记录图像队列族所有权转移的 release barrier。
        /// @param[in] cmdBuffer 用于记录 barrier 的命令缓冲区。
        /// @param[in] srcQueueFamilyIndex 当前拥有图像的队列族索引。
        /// @param[in] dstQueueFamilyIndex 即将接收图像的队列族索引。
        /// @note 当前实现只覆盖 color aspect、全部 mip 和第一个数组层。
        void addReleaseBarrier(VkCommandBuffer cmdBuffer,
                               uint32_t srcQueueFamilyIndex,
                               uint32_t dstQueueFamilyIndex);

        /// @brief 记录图像队列族所有权转移的 acquire barrier。
        /// @param[in] cmdBuffer 用于记录 barrier 的命令缓冲区。
        /// @param[in] srcQueueFamilyIndex 原拥有图像的队列族索引。
        /// @param[in] dstQueueFamilyIndex 当前接收图像的队列族索引。
        /// @note 当前实现只覆盖 color aspect、全部 mip 和第一个数组层。
        void addAcquireBarrier(VkCommandBuffer cmdBuffer,
                               uint32_t srcQueueFamilyIndex,
                               uint32_t dstQueueFamilyIndex);

        /// @brief 根据已记录布局向新布局转换整张图像，并更新布局记录。
        /// @param[in] cmdBuffer 用于记录 pipeline barrier 的命令缓冲区。
        /// @param[in] newLayout 目标图像布局。
        /// @pre @p cmdBuffer 必须处于录制状态，旧布局和新布局必须属于实现支持的集合。
        /// @note 布局成员在命令录制后立即更新，不代表 GPU 已经执行该转换。
        void transitionImageLayout(VkCommandBuffer cmdBuffer, VkImageLayout newLayout);

        /// @brief 判断图像格式是否包含深度分量。
        /// @return 包含深度分量时返回 `true`。
        bool isDepth() const;

        /// @brief 判断图像格式是否包含模板分量。
        /// @return 包含模板分量时返回 `true`。
        bool isStencil() const;

        /// @brief 获取当前格式单个像素占用的字节数。
        /// @return 由 bytesPerPixel() 计算的字节数。
        uint32_t pixelSizeInBytes() const;

        /// @brief 获取图像的 mip 层数。
        /// @return 实际创建或记录的 mip 层数。
        uint32_t numMipLevels() const;

        /// @brief 在 GPU 上通过线性 blit 生成 mipmap。
        /// @param[in] cmdBuffer 用于记录 barrier 和 blit 命令的命令缓冲区。
        /// @pre 图像格式必须支持线性过滤 blit，图像 usage 必须支持传输源和目标。
        /// @note 仅在构造时启用 mipmap 生成后生效；当前实现只处理第一个数组层。
        void generateMips(VkCommandBuffer cmdBuffer);

        /// @brief 为每个 mip 层分别创建一个图像视图。
        /// @return 按 mip 层顺序排列的图像视图句柄存储对象。
        /// @warning 返回的 `shared_ptr` 只管理句柄存储，不会调用 `vkDestroyImageView`；
        ///          当前实现也不会由 `Texture` 析构函数销毁这些视图。
        std::vector<std::shared_ptr<VkImageView>> generateViewForEachMips();

        /// @brief 获取图像的多重采样数。
        /// @return 创建图像时记录的 `VkSampleCountFlagBits`。
        VkSampleCountFlagBits VkSampleCount() const;
    private:
        /// @brief 根据二维尺寸计算完整 mip 链的层数。
        uint32_t getMipLevelsCount(uint32_t texWidth, uint32_t texHeight) const;

        /// @brief 为当前图像创建并命名一个 Vulkan 图像视图。
        VkImageView createImageView(const Context& context,
                                    VkImageViewType viewType,
                                    VkFormat format,
                                    uint32_t numMipLevels,
                                    uint32_t layers,
                                    const std::string& name = "");
    private:
        /// 非拥有引用；`Context` 必须比本对象存活更久。
        const Context& context_;

        /// 仅在自有图像模式下用于创建和销毁 VMA 图像。
        VmaAllocator vmaAllocator_ = nullptr;
        VmaAllocation vmaAllocation_ = nullptr;
        VkDeviceSize deviceSize_ = 0;
        VkImageUsageFlags usageFlags_ = 0;
        VkImageCreateFlags flags_ = 0;
        VkImageType type_ = VK_IMAGE_TYPE_2D;
        VkImage image_ = VK_NULL_HANDLE;

        /// 本对象拥有并负责销毁的主图像视图。
        VkImageView imageView_ = VK_NULL_HANDLE;

        /// 按请求的 mip 层缓存、并由本对象负责销毁的图像视图。
        std::unordered_map<uint32_t, VkImageView> imageViewFramebuffers_;
        VkFormat format_ = VK_FORMAT_UNDEFINED;
        VkExtent3D extents_;

        /// CPU 侧记录的整张图像布局。
        VkImageLayout layout_ = VK_IMAGE_LAYOUT_UNDEFINED;

        /// 为 `true` 时，析构函数通过 VMA 销毁 image_ 及其内存分配。
        bool ownsVkImage_ = false;
        uint32_t mipLevels_ = 1;
        uint32_t layerCount_ = 1;
        bool multiview_ = false;
        bool generateMips_ = false;
        VkImageViewType viewType_;
        VkSampleCountFlagBits msaaSamples_ = VK_SAMPLE_COUNT_1_BIT;
        VkImageTiling imageTiling_ = VK_IMAGE_TILING_OPTIMAL;
        std::string debugName_;
    };

}  // namespace Huli::Vulkan
