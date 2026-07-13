#pragma once

#include <any>
#include <array>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "Buffer.hpp"
#include "CommandQueueManager.hpp"
#include "Common.hpp"
#include "PhysicalDevice.hpp"
#include "Pipeline.hpp"
#include "ShaderModule.hpp"
#include "Swapchain.hpp"
#include "Utility.hpp"
#include "vk_mem_alloc.h"

namespace Huli::Vulkan
{
    class Framebuffer;
    class RenderPass;
    class Sampler;
    class Texture;

    /// @brief 在稳定存储中组装 Vulkan @c pNext 功能链。
    ///
    /// 每次调用 pushBack() 都会复制一份结构体到内部 @c std::any 数组，并将它插入当前链表头部。
    /// 因此 firstNextPtr() 返回的顺序与调用顺序相反，内部结构体会一直存活到本对象销毁。
    /// @tparam CHAIN_SIZE 最多能够保存的 Vulkan 链式结构体数量。
    /// @note 传入的结构体必须包含可写的 @c pNext 成员。
    /// @warning 链表指针指向本对象的内部存储；开始组链后不要移动本对象。
    template <size_t CHAIN_SIZE = 10> class VulkanFeatureChain
    {
    public:
        /// @brief 创建一条尚未包含任何结构体的空链。
        VulkanFeatureChain() = default;
        MOVABLE_ONLY(VulkanFeatureChain);

        /// @brief 复制并将一个 Vulkan 结构体插入 @c pNext 链表头部。
        /// @param[in] nextVulkanChainStruct 要保存的 Vulkan 链式结构体。
        /// @return 内部副本的引用，可在提交 Vulkan 创建调用前继续修改。
        /// @pre 当前保存数量必须小于 @c CHAIN_SIZE。
        auto& pushBack(auto nextVulkanChainStruct)
        {
            ASSERT(currentIndex_ < CHAIN_SIZE, "Chain is full");
            data_[currentIndex_] = nextVulkanChainStruct;

            auto& next = std::any_cast<decltype(nextVulkanChainStruct)&>(data_[currentIndex_]);

            next.pNext = std::exchange(firstNext_, &next);
            currentIndex_++;

            return next;
        }

        /// @brief 取得当前 @c pNext 链的头指针。
        /// @return 空链返回 @c nullptr，否则返回最后一次插入的结构体地址。
        /// @warning 返回指针仅在本对象未移动且仍然存活时有效。
        [[nodiscard]] void* firstNextPtr() const { return firstNext_; };
    private:
        /// 保存链式结构体副本，使创建调用期间的对象生命周期稳定。
        std::array<std::any, CHAIN_SIZE> data_;
        /// 当前实现未使用的预留根节点指针。
        VkBaseInStructure* root_ = nullptr;
        /// 下一次写入 @c data_ 的下标。
        int currentIndex_ = 0;
        /// 当前链表头；空链时为 @c nullptr。
        void* firstNext_ = VK_NULL_HANDLE;
    };

    /// @brief 管理 Vulkan instance、device、队列、VMA allocator 和可选 swapchain 的顶层上下文。
    ///
    /// 本类拥有 @c VkInstance、可选 @c VkSurfaceKHR、@c VkDevice、@c VmaAllocator、可选
    /// @c Swapchain 以及 debug messenger。物理设备和队列句柄由 Vulkan instance/device 提供，
    /// 不单独拥有。通过工厂方法创建的资源通常会借用本对象、逻辑设备或 allocator，因此必须
    /// 在本对象销毁前释放。
    ///
    /// 功能启用方法修改的是所有 Context 共享的静态创建状态。应在创建设备前调用，并由调用方
    /// 负责串行配置；已启用的位会影响之后创建的 Context，当前没有重置接口。
    /// @warning MOVABLE_ONLY 当前生成默认移动操作，无法转移并清空原始 Vulkan 句柄；在实现安全的
    ///          自定义移动操作前，不要移动 Context 对象。
    class Context final
    {
    public:
        MOVABLE_ONLY(Context);

        /// @brief 从平台窗口创建完整的 Vulkan 上下文。
        ///
        /// 构造过程会过滤可用 layer/extension，创建 instance、可选 Win32 surface 和 debug messenger，
        /// 选择物理设备，创建逻辑设备与所需队列，最后创建 VMA allocator。无论 @p requestedQueueTypes
        /// 是否包含 graphics，当前实现都会额外请求 graphics queue。
        /// @param[in] window Win32 构建中作为 @c HWND 使用的平台窗口；为 @c nullptr 时不创建 surface。
        /// @param[in] requestedLayers 希望启用的 instance layer；不可用项会被过滤。
        /// @param[in] requestedInstanceExtensions 希望启用的 instance extension；不可用项会被过滤。
        /// @param[in] requestedDeviceExtensions 希望启用的 device extension；每个候选设备会过滤不可用项。
        /// @param[in] requestedQueueTypes 除 graphics queue 外希望保留的队列类型。
        /// @param[in] printEnumerations 是否将枚举到的 layer、extension 和设备信息写入日志。
        /// @param[in] enableRayTracing 是否在查询物理设备 feature 时包含 ray tracing 功能链。
        /// @param[in] name 用于 Vulkan debug object name 的可读名称后缀。
        explicit Context(void* window,
                         const std::vector<std::string>& requestedLayers,
                         const std::vector<std::string>& requestedInstanceExtensions,
                         const std::vector<std::string>& requestedDeviceExtensions,
                         VkQueueFlags requestedQueueTypes,
                         bool printEnumerations = false,
                         bool enableRayTracing = false,
                         const std::string& name = "");

        /// @brief 仅创建 Vulkan instance，供调用方稍后指定物理设备。
        /// @param[in] appInfo 复制到上下文中的 Vulkan 应用信息；其中字符串指针必须在创建 instance 时有效。
        /// @param[in] requestedLayers 希望启用的 instance layer；不可用项会被过滤。
        /// @param[in] requestedInstanceExtensions 希望启用的 instance extension；不可用项会被过滤。
        /// @param[in] printEnumerations 是否将枚举结果写入日志。
        /// @param[in] name 预留的 debug 名称后缀；当前构造路径不会创建设备。
        /// @note 此构造函数不会创建 surface、逻辑设备、队列或 VMA allocator。
        /// @warning 当前析构函数要求逻辑设备已经创建；销毁前必须成功调用 createVkDevice()。
        explicit Context(const VkApplicationInfo& appInfo,
                         const std::vector<std::string>& requestedLayers,
                         const std::vector<std::string>& requestedInstanceExtensions,
                         bool printEnumerations = false,
                         const std::string& name = "");

        /// @brief 为仅含 instance 的上下文创建逻辑设备、队列和 VMA allocator。
        /// @param[in] vkPhysicalDevice 属于当前 instance 的物理设备句柄。
        /// @param[in] requestedDeviceExtensions 希望启用的 device extension；不可用项会被过滤。
        /// @param[in] requestedQueueTypes 除 graphics queue 外希望保留的队列类型。
        /// @param[in] name 用于逻辑设备和 instance debug object name 的可读名称后缀。
        /// @pre 当前上下文已经创建有效 instance，且尚未创建逻辑设备。
        /// @note 此路径不创建 surface，因此不能据此创建 swapchain。
        void createVkDevice(VkPhysicalDevice vkPhysicalDevice,
                            const std::vector<std::string>& requestedDeviceExtensions,
                            VkQueueFlags requestedQueueTypes,
                            const std::string& name = "");

        /// @brief 等待逻辑设备空闲并按依赖顺序销毁本上下文拥有的 Vulkan 对象。
        /// @pre 所有借用本上下文、逻辑设备或 VMA allocator 的外部资源都已销毁。
        ~Context();

        /// @brief 启用默认的 descriptor indexing 与 runtime descriptor array 功能集合。
        static void enableDefaultFeatures();

        /// @brief 启用 Vulkan 1.2 @c scalarBlockLayout 功能。
        static void enableScalarLayoutFeatures();

        /// @brief 启用 Vulkan 1.3 dynamic rendering 功能。
        static void enableDynamicRenderingFeature();

        /// @brief 启用 buffer device address 及 capture replay 功能。
        static void enableBufferDeviceAddressFeature();

        /// @brief 启用 shader draw parameters、indirect count 和 multi-draw indirect 功能。
        static void enableIndirectRenderingFeature();

        /// @brief 启用 storage buffer 16-bit access 与 shader float16 功能。
        static void enable16bitFloatFeature();

        /// @brief 启用每个 render target 独立配置 blend state 的功能。
        static void enableIndependentBlending();

        /// @brief 启用 Vulkan 1.3 maintenance4 功能。
        static void enableMaintenance4Feature();

        /// @brief 启用 Vulkan 1.3 synchronization2 功能。
        static void enableSynchronization2Feature();

        /// @brief 启用 acceleration structure、ray tracing pipeline 和 ray query 功能。
        /// @note 这里只修改 feature 位，相关 device extension 仍需在创建 Context 时请求。
        static void enableRayTracingFeatures();

        /// 所有 Context 共享的 multiview 请求标记；优先通过 enableMultiView() 修改。
        static bool enableMultiViewFlag_;
        /// @brief 请求在支持 multiview 的物理设备上启用该功能。
        static void enableMultiView();

        /// @brief 查询全局 multiview 请求标记。
        /// @return 全局请求标记为 @c true 时返回 @c true。
        static bool isMultiviewEnabled();

        /// @brief 启用 fragment density map 功能位。
        static void enableFragmentDensityMapFeatures();

        /// @brief 启用 Qualcomm fragment density map offset 功能位。
        static void enableFragmentDensityMapOffsetFeatures();

        /// @brief 取得本上下文拥有的逻辑设备句柄。
        /// @return 借用的 @c VkDevice；仅在本上下文存活期间有效。
        VkDevice device() const { return device_; }

        /// @brief 取得本上下文拥有的 Vulkan instance 句柄。
        /// @return 借用的 @c VkInstance；仅在本上下文存活期间有效。
        VkInstance instance() const { return instance_; }

        /// @brief 取得本上下文拥有的 VMA allocator。
        /// @return 借用的 @c VmaAllocator；仅在本上下文存活期间有效。
        [[nodiscard]] inline VmaAllocator memoryAllocator() const { return allocator_; }

        /// @brief 取得当前选中的物理设备封装。
        /// @return 指向内部对象的常量引用；本上下文销毁后失效。
        const PhysicalDevice& physicalDevice() const;

        /// @brief 创建并由 Context 持有 swapchain。
        /// @param[in] format swapchain image format。
        /// @param[in] colorSpace 与 @p format 配套的 surface color space。
        /// @param[in] presentMode 图像提交到 presentation engine 的模式。
        /// @param[in] extent swapchain 图像尺寸。
        /// @pre 当前上下文必须已从有效窗口创建 surface。
        /// @note 再次调用会销毁并替换先前持有的 swapchain。
        void createSwapchain(VkFormat format,
                             VkColorSpaceKHR colorSpace,
                             VkPresentModeKHR presentMode,
                             const VkExtent2D& extent);

        /// @brief 取得当前由 Context 持有的 swapchain。
        /// @return 尚未调用 createSwapchain() 时返回 @c nullptr，否则返回非拥有指针。
        /// @warning 再次创建 swapchain 或销毁 Context 后，返回指针失效。
        Swapchain* swapchain() const;

        /// @brief 取得指定下标的 graphics queue。
        /// @param[in] index @c graphicsQueues_ 中的队列下标。
        /// @return 借用的 @c VkQueue；其生命周期由逻辑设备决定。
        /// @pre @p index 必须位于已经保留的 graphics queue 范围内；当前访问器不执行边界检查。
        VkQueue graphicsQueue(int index = 0) const { return graphicsQueues_[index]; }

        /// @brief 创建由 VMA 分配且支持主机随机访问的 buffer。
        /// @param[in] size buffer 大小，单位为字节。
        /// @param[in] flags Vulkan buffer usage 标志。
        /// @param[in] memoryUsage VMA 的内存用途提示。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 Buffer；它借用本 Context 和 allocator，必须先于本 Context 销毁。
        std::shared_ptr<Buffer> createBuffer(size_t size,
                                             VkBufferUsageFlags flags,
                                             VmaMemoryUsage memoryUsage,
                                             const std::string& name = "") const;

        /// @brief 创建要求 host visible、coherent 且 cached 的 CPU-to-GPU buffer。
        /// @param[in] size buffer 大小，单位为字节。
        /// @param[in] flags Vulkan buffer usage 标志。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 Buffer；它借用本 Context 和 allocator，必须先于本 Context 销毁。
        std::shared_ptr<Buffer> createPersistentBuffer(size_t size,
                                                       VkBufferUsageFlags flags,
                                                       const std::string& name = "") const;

        /// @brief 创建一块映射的、面向顺序写入的 CPU staging buffer。
        /// @param[in] size buffer 大小，单位为字节。
        /// @param[in] usage 预期的 usage；当前实现未使用该参数并固定创建 @c VK_BUFFER_USAGE_TRANSFER_SRC_BIT。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 staging Buffer，必须先于本 Context 销毁。
        std::shared_ptr<Buffer> createStagingBuffer(VkDeviceSize size,
                                                    VkBufferUsageFlags usage,
                                                    const std::string& name = "") const;

        /// @brief 创建与目标 GPU buffer 关联的 staging buffer。
        /// @param[in] size staging buffer 大小，单位为字节。
        /// @param[in] usage staging buffer usage；Buffer 构造函数会补充 transfer source usage。
        /// @param[in] actualBuffer 接收复制数据的目标 Buffer；不转移所有权。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 staging Buffer，必须先于本 Context 销毁。
        /// @pre @p actualBuffer 非空，且与本 Context 使用兼容的逻辑设备。
        std::shared_ptr<Buffer> createStagingBuffer(VkDeviceSize size,
                                                    VkBufferUsageFlags usage,
                                                    Buffer* actualBuffer,
                                                    const std::string& name = "") const;

        /// @brief 通过临时 staging buffer 将 CPU 数据复制到 GPU buffer。
        ///
        /// 本方法只记录 copy command，并把 staging buffer 交给 @p queueMgr 延迟释放；不会结束、提交或等待
        /// @p commandBuffer。
        /// @param[in,out] queueMgr 管理 command buffer 提交和 staging buffer 延迟释放的队列管理器。
        /// @param[in] commandBuffer 正在记录且允许执行 transfer copy 的 command buffer。
        /// @param[in] gpuBuffer 目标 GPU Buffer；不转移所有权。
        /// @param[in] data 要上传的 CPU 数据起始地址。
        /// @param[in] totalSize 上传字节数。
        /// @param[in] gpuBufferOffset 目标 buffer 内的起始字节偏移。
        /// @pre 所有对象必须属于兼容的逻辑设备，且目标范围足以容纳 @p totalSize 字节。
        void uploadToGPUBuffer(CommandQueueManager& queueMgr,
                               VkCommandBuffer commandBuffer,
                               Buffer* gpuBuffer,
                               const void* data,
                               long totalSize,
                               uint64_t gpuBufferOffset = 0) const;

        /// @brief 创建由 VMA 分配并由 Texture 管理的 Vulkan image。
        /// @param[in] type image dimensionality。
        /// @param[in] format image texel format。
        /// @param[in] flags Vulkan image create flags。
        /// @param[in] usageFlags Vulkan image usage flags。
        /// @param[in] extents image 的宽、高、深。
        /// @param[in] numMipLevels mip level 数量。
        /// @param[in] layerCount array layer 数量。
        /// @param[in] memoryFlags 所需 Vulkan memory property flags。
        /// @param[in] generateMips 是否允许该 Texture 的上传路径生成 mipmap。
        /// @param[in] msaaSamples image sample count。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 Texture；Texture 借用本 Context，必须先于本 Context 销毁。
        std::shared_ptr<Texture> createTexture(VkImageType type,
                                               VkFormat format,
                                               VkImageCreateFlags flags,
                                               VkImageUsageFlags usageFlags,
                                               VkExtent3D extents,
                                               uint32_t numMipLevels,
                                               uint32_t layerCount,
                                               VkMemoryPropertyFlags memoryFlags,
                                               bool generateMips = false,
                                               VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT,
                                               const std::string& name = "") const;

        /// @brief 创建未启用 compare operation 的 sampler。
        /// @param[in] minFilter 纹理缩小时的过滤方式。
        /// @param[in] magFilter 纹理放大时的过滤方式。
        /// @param[in] addressModeU U 轴寻址方式。
        /// @param[in] addressModeV V 轴寻址方式。
        /// @param[in] addressModeW W 轴寻址方式。
        /// @param[in] maxLod 可采样的最大 mip level。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 Sampler；其 Vulkan sampler 必须先于本 Context 的逻辑设备销毁。
        std::shared_ptr<Sampler> createSampler(VkFilter minFilter,
                                               VkFilter magFilter,
                                               VkSamplerAddressMode addressModeU,
                                               VkSamplerAddressMode addressModeV,
                                               VkSamplerAddressMode addressModeW,
                                               float maxLod,
                                               const std::string& name = "") const;

        /// @brief 创建可选启用 compare operation 的 sampler。
        /// @param[in] minFilter 纹理缩小时的过滤方式。
        /// @param[in] magFilter 纹理放大时的过滤方式。
        /// @param[in] addressModeU U 轴寻址方式。
        /// @param[in] addressModeV V 轴寻址方式。
        /// @param[in] addressModeW W 轴寻址方式。
        /// @param[in] maxLod 可采样的最大 mip level。
        /// @param[in] compareEnable 是否启用采样比较。
        /// @param[in] compareOp 启用比较时使用的 Vulkan compare operation。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 Sampler；其 Vulkan sampler 必须先于本 Context 的逻辑设备销毁。
        std::shared_ptr<Sampler> createSampler(VkFilter minFilter,
                                               VkFilter magFilter,
                                               VkSamplerAddressMode addressModeU,
                                               VkSamplerAddressMode addressModeV,
                                               VkSamplerAddressMode addressModeW,
                                               float maxLod,
                                               bool compareEnable,
                                               VkCompareOp compareOp,
                                               const std::string& name = "") const;

        /// @brief 为一个 graphics queue 创建 command pool、command buffer 和同步资源管理器。
        /// @param[in] count 管理器创建的 command buffer 数量。
        /// @param[in] concurrentNumCommands 允许并行处于提交状态的 command 数量。
        /// @param[in] name debug object name 后缀。
        /// @param[in] graphicsQueueIndex 使用的 graphics queue 下标；@c -1 表示第一个队列。
        /// @return 拥有 command pool、command buffer 和 fence 的 CommandQueueManager。
        /// @pre @p graphicsQueueIndex 必须为 @c -1 或有效的非负下标，且 Context 必须比返回对象存活更久。
        CommandQueueManager createGraphicsCommandQueue(uint32_t count,
                                                       uint32_t concurrentNumCommands,
                                                       const std::string& name = "",
                                                       int graphicsQueueIndex = -1);

        /// @brief 从 SPIR-V 文件创建使用默认 entry point 的 shader module。
        /// @param[in] filePath SPIR-V 二进制文件路径。
        /// @param[in] stages 此模块对应的 shader stage。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 ShaderModule；它借用本 Context，必须先于本 Context 销毁。
        std::shared_ptr<ShaderModule> createShaderModule(const std::string& filePath,
                                                         VkShaderStageFlagBits stages,
                                                         const std::string& name = "");

        /// @brief 从 SPIR-V 文件和显式 entry point 创建 shader module。
        /// @param[in] filePath SPIR-V 二进制文件路径。
        /// @param[in] entryPoint shader entry point 名称。
        /// @param[in] stages 此模块对应的 shader stage。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 ShaderModule；它借用本 Context，必须先于本 Context 销毁。
        std::shared_ptr<ShaderModule> createShaderModule(const std::string& filePath,
                                                         const std::string& entryPoint,
                                                         VkShaderStageFlagBits stages,
                                                         const std::string& name = "");

        /// @brief 从内存中的 SPIR-V 字节码创建 shader module。
        /// @param[in] shader SPIR-V 二进制字节。
        /// @param[in] entryPoint shader entry point 名称。
        /// @param[in] stages 此模块对应的 shader stage。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 ShaderModule；它借用本 Context，必须先于本 Context 销毁。
        std::shared_ptr<ShaderModule> createShaderModule(const std::vector<char>& shader,
                                                         const std::string& entryPoint,
                                                         VkShaderStageFlagBits stages,
                                                         const std::string& name = "");

        /// @brief 根据 descriptor 和兼容 render pass 创建 graphics pipeline。
        /// @param[in] desc graphics pipeline 的完整描述。
        /// @param[in] renderPass pipeline 所兼容的 render pass；不转移所有权。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 Pipeline；它借用本 Context，必须先于本 Context 销毁。
        std::shared_ptr<Pipeline> createGraphicsPipeline(const Pipeline::GraphicsPipelineDescriptor& desc,
                                                         VkRenderPass renderPass,
                                                         const std::string& name = "");

        /// @brief 根据 descriptor 创建 compute pipeline。
        /// @param[in] desc compute pipeline 的完整描述。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 Pipeline；它借用本 Context，必须先于本 Context 销毁。
        std::shared_ptr<Pipeline> createComputePipeline(const Pipeline::ComputePipelineDescriptor& desc,
                                                        const std::string& name = "");

        /// @brief 根据 descriptor 创建 ray tracing pipeline。
        /// @param[in] desc ray tracing pipeline 的完整描述。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 Pipeline；它借用本 Context，必须先于本 Context 销毁。
        /// @pre 设备创建前已经请求并成功启用所需 ray tracing extension 和 feature。
        std::shared_ptr<Pipeline> createRayTracingPipeline(const Pipeline::RayTracingPipelineDescriptor& desc,
                                                           const std::string& name = "");

        /// @brief 为一个 transfer queue 创建 command pool、command buffer 和同步资源管理器。
        /// @param[in] count 管理器创建的 command buffer 数量。
        /// @param[in] concurrentNumCommands 允许并行处于提交状态的 command 数量。
        /// @param[in] name debug object name 后缀。
        /// @param[in] transferQueueIndex 使用的 transfer queue 下标；@c -1 表示第一个队列。
        /// @return 拥有 command pool、command buffer 和 fence 的 CommandQueueManager。
        /// @pre 创建 Context 时必须请求 transfer queue；下标必须为 @c -1 或有效非负值，且 Context
        ///      必须比返回对象存活更久。
        CommandQueueManager createTransferCommandQueue(uint32_t count,
                                                       uint32_t concurrentNumCommands,
                                                       const std::string& name,
                                                       int transferQueueIndex = -1);

        /// @brief 根据 attachment 列表创建 render pass。
        /// @param[in] attachments 主 attachment；顺序决定 attachment index。
        /// @param[in] loadOp 与 @p attachments 一一对应的 load operation。
        /// @param[in] storeOp 与 @p attachments 一一对应的 store operation。
        /// @param[in] layout 与 @p attachments 一一对应的最终 image layout。
        /// @param[in] bindPoint subpass 使用的 pipeline bind point。
        /// @param[in] resolveAttachments 可选 resolve attachment 列表。
        /// @param[in] name debug object name 后缀。
        /// @return 共享拥有的 RenderPass；其 Vulkan 对象必须先于本 Context 的逻辑设备销毁。
        std::shared_ptr<RenderPass> createRenderPass(
            const std::vector<std::shared_ptr<Texture>>& attachments,
            const std::vector<VkAttachmentLoadOp>& loadOp,
            const std::vector<VkAttachmentStoreOp>& storeOp,
            const std::vector<VkImageLayout>& layout,
            VkPipelineBindPoint bindPoint,
            const std::vector<std::shared_ptr<Texture>>& resolveAttachments = {},
            const std::string& name = "") const;

        /// @brief 根据颜色、深度和模板 image view 创建 framebuffer。
        /// @param[in] renderPass framebuffer 兼容的 Vulkan render pass；不转移所有权。
        /// @param[in] colorAttachments 按顺序写入 framebuffer 的颜色 attachment。
        /// @param[in] depthAttachment 可选深度 attachment。
        /// @param[in] stencilAttachment 可选模板 attachment。
        /// @param[in] name debug object name 后缀。
        /// @return 独占拥有的 Framebuffer；其 Vulkan 对象必须先于本 Context 的逻辑设备销毁。
        /// @pre @p renderPass 和所有 attachment image view 必须在返回的 Framebuffer 存活期间保持有效。
        std::unique_ptr<Framebuffer> createFramebuffer(VkRenderPass renderPass,
                                                       const std::vector<std::shared_ptr<Texture>>& colorAttachments,
                                                       std::shared_ptr<Texture> depthAttachment,
                                                       std::shared_ptr<Texture> stencilAttachment,
                                                       const std::string& name = "") const;

        /// @brief 将 VMA 当前的完整统计信息写入文件。
        /// @param[in] fileName 输出文件路径。
        /// @pre VMA allocator 已成功创建。
        /// @see vmaBuildStatsString()
        void dumpMemoryStats(const std::string& fileName) const;

        /// @brief 在启用 @c VK_EXT_debug_utils 时为 Vulkan 对象设置可读名称。
        /// @tparam T Vulkan handle 类型。
        /// @param[in] handle 要命名的 Vulkan handle。
        /// @param[in] type 与 @p handle 匹配的 @c VkObjectType。
        /// @param[in] name 调试工具中显示的名称。
        /// @note 未编译或未启用 @c VK_EXT_debug_utils 时，本方法不执行任何操作。
        template <typename T> void setVkObjectname(T handle, VkObjectType type, const std::string& name) const
        {
#if defined(VK_EXT_debug_utils)
            if (enabledInstanceExtensions_.find(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != enabledInstanceExtensions_.end())
            {
                const VkDebugUtilsObjectNameInfoEXT objectNameInfo = {
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                    .objectType = type,
                    .objectHandle = reinterpret_cast<uint64_t>(handle),
                    .pObjectName = name.c_str(),
                };
                VK_CHECK(vkSetDebugUtilsObjectNameEXT(device_, &objectNameInfo));
            }
#else
            (void)handle;
            (void)type;
            (void)name;
#endif
        }

        /// @brief 在 command buffer 中开始一个 debug utils label 区域。
        /// @param[in] commandBuffer 正在记录的 command buffer。
        /// @param[in] name 调试工具中显示的 label 名称。
        /// @param[in] color label 的 RGBA 调试颜色。
        /// @pre Windows 构建中调用时必须已经启用 @c VK_EXT_debug_utils，并与 endDebugUtilsLabel() 成对。
        /// @note 当前实现在非 Windows 或未编译 @c VK_EXT_debug_utils 时不执行任何操作。
        void beginDebugUtilsLabel(VkCommandBuffer commandBuffer, const std::string& name, const glm::vec4& color) const;

        /// @brief 结束最近开始的 debug utils label 区域。
        /// @param[in] commandBuffer 与 beginDebugUtilsLabel() 相同且正在记录的 command buffer。
        /// @pre Windows 构建中调用时必须已经启用 @c VK_EXT_debug_utils，并存在未结束的 label。
        /// @note 当前实现在非 Windows 或未编译 @c VK_EXT_debug_utils 时不执行任何操作。
        void endDebugUtilsLabel(VkCommandBuffer commandBuffer) const;
    private:
        /// @brief 使用当前 instance、物理设备和逻辑设备创建 VMA allocator。
        void createMemoryAllocator();

        /// @brief 枚举当前 Vulkan loader 提供的 instance layer 名称。
        /// @param[in] printEnumerations 是否将枚举结果写入日志。
        /// @return 可用 layer 名称列表。
        [[nodiscard]] static std::vector<std::string> enumerateInstanceLayers(bool printEnumerations_ = false);

        /// @brief 枚举当前 Vulkan loader 提供的 instance extension 名称。
        /// @return 可用 extension 名称列表。
        [[nodiscard]] std::vector<std::string> enumerateInstanceExtensions();

        /// @brief 将 instance 枚举到的物理设备封装为 PhysicalDevice 候选项。
        /// @param[in] requestedExtensions 希望为每个候选设备启用的 device extension；不可用项会被过滤。
        /// @param[in] enableRayTracing 是否在 feature 查询链中包含 ray tracing 节点。
        /// @return 可供 choosePhysicalDevice() 选择的物理设备列表。
        [[nodiscard]] std::vector<PhysicalDevice> enumeratePhysicalDevices(
            const std::vector<std::string>& requestedExtensions,
            bool enableRayTracing) const;

        /// @brief 从候选列表中选择物理设备。
        /// @param[in] devices 候选设备列表。
        /// @param[in] deviceExtensions 当前实现未使用的 device extension 列表。
        /// @return 名称包含 @c NVIDIA 的首个设备；不存在时返回列表首项。
        [[nodiscard]] PhysicalDevice choosePhysicalDevice(std::vector<PhysicalDevice>&& devices,
                                                          const std::vector<std::string>& deviceExtensions) const;
    private:
        /// 传给 @c vkCreateInstance 的应用信息副本；默认请求 Vulkan 1.3。
        const VkApplicationInfo applicationInfo_ = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Modern Vulkan Cookbook",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3,
        };
        /// 本对象拥有的 Vulkan instance。
        VkInstance instance_ = VK_NULL_HANDLE;
        /// 当前选中的物理设备封装；其中 Vulkan handle 不需要单独销毁。
        PhysicalDevice physicalDevice_;
        /// 本对象拥有的 Vulkan logical device。
        VkDevice device_ = VK_NULL_HANDLE;
        /// 本对象拥有、并依赖 instance/physical device/logical device 的 VMA allocator。
        VmaAllocator allocator_ = nullptr;
        /// 是否在枚举 layer、extension 和设备时输出诊断信息。
        bool printEnumerations_ = false;
        /// 可选的平台 presentation surface；由本对象拥有。
        VkSurfaceKHR surface_ = VK_NULL_HANDLE;
        /// 为 surface format 缓存预留的容器；当前实现尚未填充。
        std::vector<VkSurfaceFormatKHR> surfaceFormats_;
        /// 用于 presentation 的主队列句柄；由 logical device 拥有。
        VkQueue presentationQueue_ = VK_NULL_HANDLE;

        /// 所有 Context 共享的 Vulkan 1.0 feature 请求。
        static VkPhysicalDeviceFeatures physicalDeviceFeatures_;
        /// 所有 Context 共享的 Vulkan 1.1 feature 请求。
        static VkPhysicalDeviceVulkan11Features enable11Features_;
        /// 所有 Context 共享的 Vulkan 1.2 feature 请求。
        static VkPhysicalDeviceVulkan12Features enable12Features_;
        /// 所有 Context 共享的 Vulkan 1.3 feature 请求。
        static VkPhysicalDeviceVulkan13Features enable13Features_;

        /// 所有 Context 共享的 acceleration structure feature 请求。
        static VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures_;
        /// 所有 Context 共享的 ray tracing pipeline feature 请求。
        static VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures_;
        /// 所有 Context 共享的 ray query feature 请求。
        static VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures_;
        /// 预留的 multiview feature 节点；当前设备创建链未使用该成员。
        static VkPhysicalDeviceMultiviewFeatures multiviewFeatures_;
        /// 所有 Context 共享的 fragment density map feature 请求。
        static VkPhysicalDeviceFragmentDensityMapFeaturesEXT fragmentDensityMapFeatures_;
        /// 所有 Context 共享的 Qualcomm fragment density map offset feature 请求。
        static VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM fragmentDensityMapOffsetFeatures_;

        /// 可供 graphics 工作使用的队列句柄；由 logical device 拥有。
        std::vector<VkQueue> graphicsQueues_;
        /// 可供异步 compute 工作使用的队列句柄；由 logical device 拥有。
        std::vector<VkQueue> computeQueues_;
        /// 可供异步 transfer 工作使用的队列句柄；由 logical device 拥有。
        std::vector<VkQueue> transferQueues_;
        /// 可供 sparse binding 工作使用的队列句柄；由 logical device 拥有。
        std::vector<VkQueue> sparseQueues_;

        /// Context 当前独占拥有的可选 swapchain。
        std::unique_ptr<Swapchain> swapchain_;
        /// 实际启用的 instance layer 名称集合。
        std::unordered_set<std::string> enabledLayers_;
        /// 实际启用的 instance extension 名称集合。
        std::unordered_set<std::string> enabledInstanceExtensions_;
#if defined(VK_EXT_debug_utils)
        /// 启用 debug utils extension 时由本对象拥有的 debug messenger。
        VkDebugUtilsMessengerEXT messenger_ = VK_NULL_HANDLE;
#endif
    };

}  // namespace Huli::Vulkan
