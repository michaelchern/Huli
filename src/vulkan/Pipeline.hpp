#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <span>
#include <unordered_map>

#include "Common.hpp"
#include "Utility.hpp"

/// @file Pipeline.hpp
/// @brief 声明图形、计算和光线追踪 Vulkan pipeline 及其 descriptor 资源的封装。

namespace Huli::Vulkan
{

    class Context;
    class Buffer;
    class RenderPass;
    class Sampler;
    class ShaderModule;
    class Texture;

    /// @brief 为 bindless descriptor 数量预留的上限；当前 `Pipeline` 实现尚未使用该常量。
    constexpr uint32_t MAX_DESC_BINDLESS = 1000;

    /// @brief 创建并管理 Vulkan pipeline、pipeline layout 及其 descriptor 资源。
    ///
    /// 构造函数根据传入的描述创建图形、计算或光线追踪 pipeline。对象拥有创建得到的
    /// `VkPipeline`、`VkPipelineLayout`、`VkDescriptorPool` 和 `VkDescriptorSetLayout`，并在析构时销毁它们。
    ///
    /// @note `Context`、`VkRenderPass` 以及通过 `bindResource()` 传入的资源均不由本对象拥有；调用方必须保证
    ///       它们在相应的创建、descriptor 更新和 GPU 使用期间保持有效。
    /// @note descriptor 描述中的 `ShaderModule` 以 `std::weak_ptr` 保存，构造 pipeline 时必须仍能成功锁定。
    /// @note 本类包含延迟 descriptor 写入队列，但只有部分路径使用互斥锁，不保证整体线程安全。
    /// @note `std::mutex` 使该类型不可复制且不可移动。
    class Pipeline final
    {
    public:
        /// @brief 描述一个 descriptor set 编号及其 layout bindings。
        /// @note `set_` 直接作为 Vulkan set 编号使用；图形 pipeline 的当前实现要求编号可用于索引
        ///       `[0, sets_.size())` 范围内的 layout 数组。
        struct SetDescriptor
        {
            /// Vulkan descriptor set 编号。
            uint32_t set_;

            /// 该 set 中的 descriptor binding 描述。
            std::vector<VkDescriptorSetLayoutBinding> bindings_;
        };

        /// @brief 在 `VkExtent2D` 与 `VkViewport` 之间提供轻量转换。
        /// @note 由 extent 创建时，viewport 原点为 `(0, 0)`，深度范围为 `[0, 1]`。
        struct ViewPort
        {
            /// @brief 根据二维范围创建 viewport。
            /// @param[in] extents viewport 的宽度和高度。
            ViewPort(const VkExtent2D& extents) { viewport_ = fromExtents(extents); }

            ViewPort() = default;
            ViewPort(const ViewPort&) = default;
            ViewPort& operator=(const ViewPort&) = default;

            /// @brief 直接保存已有的 Vulkan viewport。
            /// @param[in] viewport 要保存的 viewport。
            ViewPort(const VkViewport& viewport)
                : viewport_(viewport)
            {
            }

            /// @brief 以已有 Vulkan viewport 替换当前值。
            /// @param[in] viewport 要保存的 viewport。
            /// @return 当前对象。
            ViewPort& operator=(const VkViewport& viewport)
            {
                viewport_ = viewport;
                return *this;
            }

            /// @brief 根据二维范围替换当前 viewport。
            /// @param[in] extents viewport 的宽度和高度。
            /// @return 当前对象。
            ViewPort& operator=(const VkExtent2D& extents)
            {
                viewport_ = fromExtents(extents);
                return *this;
            }

            /// @brief 获取 viewport 宽度和高度的绝对值。
            /// @return 转换为无符号整数的二维范围。
            VkExtent2D toVkExtents()
            {
                return VkExtent2D{static_cast<uint32_t>(std::abs(viewport_.width)),
                                  static_cast<uint32_t>(std::abs(viewport_.height))};
            }

            /// @brief 获取保存的 Vulkan viewport。
            /// @return 当前保存的 `VkViewport` 副本。
            VkViewport toVkViewPort() { return viewport_; }
        private:
            /// @brief 将二维范围转换为默认原点和深度范围的 viewport。
            VkViewport fromExtents(const ::VkExtent2D& extents)
            {
                VkViewport v;
                v.x = 0;
                v.y = 0;
                v.width = extents.width;
                v.height = extents.height;
                v.minDepth = 0.0;
                v.maxDepth = 1.0;
                return v;
            }

            VkViewport viewport_ = {};
        };

        /// @brief 汇总创建图形 pipeline 所需的 shader、layout 和固定功能状态。
        ///
        /// `vertexInputCreateInfo` 及 specialization data 中的裸指针只在构造 pipeline 时读取；调用方必须保证
        /// 相应后备存储至少存活到 `Pipeline` 构造完成。
        struct GraphicsPipelineDescriptor
        {
            /// 按 Vulkan set 编号描述的 descriptor set layouts。
            std::vector<SetDescriptor> sets_;

            /// 创建期间必须仍然存活的 vertex shader。
            std::weak_ptr<ShaderModule> vertexShader_;

            /// 创建期间必须仍然存活的 fragment shader。
            std::weak_ptr<ShaderModule> fragmentShader_;

            /// pipeline layout 使用的 push constant ranges。
            std::vector<VkPushConstantRange> pushConstants_;

            /// 创建时启用的动态状态。
            std::vector<VkDynamicState> dynamicStates_;

            /// 为 `true` 时通过 `VkPipelineRenderingCreateInfo` 使用 dynamic rendering。
            bool useDynamicRendering_ = false;

            /// dynamic rendering 的 color attachment formats，同时决定自动生成的 blend attachment 数量。
            std::vector<VkFormat> colorTextureFormats;

            /// dynamic rendering 的 depth attachment format。
            VkFormat depthTextureFormat = VK_FORMAT_UNDEFINED;

            /// dynamic rendering 的 stencil attachment format。
            VkFormat stencilTextureFormat = VK_FORMAT_UNDEFINED;

            /// input assembly 使用的 primitive topology。
            VkPrimitiveTopology primitiveTopology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

            /// rasterization sample count。
            VkSampleCountFlagBits sampleCount = VK_SAMPLE_COUNT_1_BIT;

            /// rasterizer 的 cull mode。
            VkCullModeFlagBits cullMode = VK_CULL_MODE_BACK_BIT;

            /// rasterizer 的 front-face winding。
            VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

            /// 创建时使用的静态 viewport；scissor 由其 extent 派生。
            ViewPort viewport;

            /// 自动生成 blend attachment 时是否启用 alpha blending。
            bool blendEnable = false;

            /// 预留的 blend attachment 数量；当前实现不读取该字段。
            uint32_t numberBlendAttachments = 0u;

            /// 是否启用 depth test。
            bool depthTestEnable = true;

            /// 是否启用 depth write。
            bool depthWriteEnable = true;

            /// depth comparison operation。
            VkCompareOp depthCompareOperation = VK_COMPARE_OP_LESS;

            /// vertex input state；其内部指针指向的描述数组必须在构造期间有效。
            VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = 0,
                .vertexAttributeDescriptionCount = 0,
            };

            /// vertex shader specialization map entries。
            std::vector<VkSpecializationMapEntry> vertexSpecConstants_;

            /// fragment shader specialization map entries。
            std::vector<VkSpecializationMapEntry> fragmentSpecConstants_;

            /// vertex specialization data 的非拥有指针。
            void* vertexSpecializationData = nullptr;

            /// fragment specialization data 的非拥有指针。
            void* fragmentSpecializationData = nullptr;

            /// 显式提供的 blend attachment states；非空时优先于自动生成的状态。
            std::vector<VkPipelineColorBlendAttachmentState> blendAttachmentStates_;
        };

        /// @brief 汇总创建计算 pipeline 所需的 shader、layout 和 specialization 状态。
        /// @warning 当前实现虽然组装了 `VkSpecializationInfo`，但未将其连接到 shader stage，
        ///          因此 `specializationConsts_` 和 `specializationData_` 暂不会影响创建结果。
        struct ComputePipelineDescriptor
        {
            /// 按 Vulkan set 编号描述的 descriptor set layouts。
            std::vector<SetDescriptor> sets_;

            /// 创建期间必须仍然存活的 compute shader。
            std::weak_ptr<ShaderModule> computeShader_;

            /// pipeline layout 使用的 push constant ranges。
            std::vector<VkPushConstantRange> pushConstants_;

            /// compute shader specialization map entries。
            std::vector<VkSpecializationMapEntry> specializationConsts_;

            /// specialization data 的非拥有指针；后备存储必须至少存活到构造完成。
            void* specializationData_ = nullptr;
        };

        /// @brief 汇总创建光线追踪 pipeline 所需的 shader groups 和 layout 状态。
        struct RayTracingPipelineDescriptor
        {
            /// 按 Vulkan set 编号描述的 descriptor set layouts。
            std::vector<SetDescriptor> sets_;

            /// ray generation shader；创建期间必须仍然存活。
            std::weak_ptr<ShaderModule> rayGenShader_;

            /// miss shaders；每个元素在创建期间都必须仍能成功锁定。
            std::vector<std::weak_ptr<ShaderModule>> rayMissShaders_;

            /// closest-hit shaders；每个元素在创建期间都必须仍能成功锁定。
            std::vector<std::weak_ptr<ShaderModule>> rayClosestHitShaders_;

            /// pipeline layout 使用的 push constant ranges。
            std::vector<VkPushConstantRange> pushConstants_;

            // Add specialization const, but they are needed per shaderModule?
        };

        /// @brief 创建图形 pipeline。
        /// @param[in] context 非拥有上下文指针；必须非空并比本对象存活更久。
        /// @param[in] desc 图形 pipeline 描述；其中所有外部指针在构造期间必须有效。
        /// @param[in] renderPass 非拥有 render pass 句柄；传统 render pass 路径下必须有效并与后续渲染兼容。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre @p desc 中的 vertex 和 fragment shader 必须仍能从 `std::weak_ptr` 锁定。
        /// @pre 使用 dynamic rendering 时，attachment formats 与设备 feature 必须满足 Vulkan 要求。
        explicit Pipeline(const Context* context,
                          const GraphicsPipelineDescriptor& desc,
                          VkRenderPass renderPass,
                          const std::string& name = "");

        /// @brief 创建计算 pipeline。
        /// @param[in] context 非拥有上下文指针；必须非空并比本对象存活更久。
        /// @param[in] desc 计算 pipeline 描述；其中所有外部指针在构造期间必须有效。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre @p desc 中的 compute shader 必须仍能从 `std::weak_ptr` 锁定。
        explicit Pipeline(const Context* context, const ComputePipelineDescriptor& desc, const std::string& name = "");

        /// @brief 创建光线追踪 pipeline。
        /// @param[in] context 非拥有上下文指针；必须非空并比本对象存活更久。
        /// @param[in] desc 光线追踪 pipeline 描述。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre 所有 shader 必须在构造期间保持有效，且逻辑设备已启用所需的 ray-tracing 扩展和 feature。
        /// @note 当前实现将最大递归深度固定为 `10`。
        explicit Pipeline(const Context* context,
                          const RayTracingPipelineDescriptor& desc,
                          const std::string& name = "");

        /// @brief 销毁本对象拥有的 pipeline、layout、descriptor pool 和 descriptor set layouts。
        /// @pre `Context` 及其逻辑设备必须仍然有效，且使用这些资源的 GPU 操作已经结束。
        ~Pipeline();

        /// @brief 检查底层 pipeline 句柄是否为空。
        /// @return 当前实现仅在 `vkPipeline_ == VK_NULL_HANDLE` 时返回 `true`。
        /// @warning 函数名与当前返回语义相反；此处记录现有行为，不表示句柄有效。
        bool valid() const { return vkPipeline_ == VK_NULL_HANDLE; }

        /// @brief 获取本对象拥有的 Vulkan pipeline 句柄。
        /// @return 调用方不得销毁的 `VkPipeline`。
        VkPipeline vkPipeline() const;

        /// @brief 获取本对象拥有的 Vulkan pipeline layout 句柄。
        /// @return 调用方不得销毁的 `VkPipelineLayout`。
        VkPipelineLayout vkPipelineLayout() const;

        /// @brief 将 offset 为 `0` 的 push constant 更新命令记录到命令缓冲区。
        /// @param[in] commandBuffer 处于录制状态的命令缓冲区。
        /// @param[in] flags 接收该范围的 shader stages。
        /// @param[in] size 从 @p data 读取的字节数。
        /// @param[in] data push constant 数据的有效指针。
        /// @pre stage flags、大小和数据必须与构造 pipeline layout 时声明的 range 兼容。
        void updatePushConstant(VkCommandBuffer commandBuffer,
                                VkShaderStageFlags flags,
                                uint32_t size,
                                const void* data);

        /// @brief 记录 pipeline 绑定命令，并提交当前排队的 descriptor 写入。
        /// @param[in] commandBuffer 处于录制状态的命令缓冲区。
        /// @note 此函数不会调用 `bindDescriptorSets()`。
        void bind(VkCommandBuffer commandBuffer);

        /// @brief 在 binding `0`、offset `0` 处记录单个 vertex buffer 绑定。
        /// @param[in] commandBuffer 处于录制状态的命令缓冲区。
        /// @param[in] vertexBuffer 要绑定的 vertex buffer 句柄。
        void bindVertexBuffer(VkCommandBuffer commandBuffer, VkBuffer vertexBuffer);

        /// @brief 在 offset `0` 处以 `VK_INDEX_TYPE_UINT32` 记录 index buffer 绑定。
        /// @param[in] commandBuffer 处于录制状态的命令缓冲区。
        /// @param[in] indexBuffer 要绑定的 index buffer 句柄。
        void bindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer indexBuffer);

        /// @brief 描述要为某个 set layout 分配的 descriptor set 数量及调试名称。
        struct SetAndCount
        {
            /// 已在 pipeline 描述中声明的 set 编号。
            uint32_t set_;

            /// 要分配的 descriptor set 数量。
            uint32_t count_;

            /// 用于生成每个 descriptor set 调试名称的前缀。
            std::string name_;
        };

        /// @brief 按请求惰性创建 descriptor pool，并为各 layout 分配 descriptor sets。
        /// @param[in] setAndCount 每个 set layout 的分配数量和调试名称。
        /// @pre 每个 `set_` 都必须已在 pipeline 描述中声明。
        void allocateDescriptors(const std::vector<SetAndCount>& setAndCount);

        /// @brief 指定要绑定的 set 编号及其已分配实例索引。
        struct SetAndBindingIndex
        {
            /// Vulkan set 编号。
            uint32_t set;

            /// `allocateDescriptors()` 为该 set 创建的实例索引。
            uint32_t bindIdx;
        };

        /// @brief 逐项记录 descriptor set 绑定命令。
        /// @param[in] commandBuffer 处于录制状态的命令缓冲区。
        /// @param[in] sets 要绑定的 set 编号和实例索引。
        /// @pre 每个 set 及实例索引都必须已经成功分配。
        void bindDescriptorSets(VkCommandBuffer commandBuffer, const std::vector<SetAndBindingIndex>& sets);

        /// @brief 为立即更新 descriptor set 的辅助函数汇总单个 binding 的资源。
        /// @note `textures_` 和 `samplers_` 是非拥有视图，仅在调用对应更新函数期间读取。
        struct SetBindings
        {
            /// 预留的 set 编号；当前更新辅助函数不读取该字段。
            uint32_t set_ = 0;

            /// 目标 binding 编号。
            uint32_t binding_ = 0;

            /// sampled-image 资源视图。
            std::span<std::shared_ptr<Texture>> textures_;

            /// sampler 资源视图。
            std::span<std::shared_ptr<Sampler>> samplers_;

            /// buffer descriptor 使用的资源。
            std::shared_ptr<Buffer> buffer;

            /// 预留的 descriptor set 实例索引；当前更新辅助函数不读取该字段。
            uint32_t index_ = 0;

            /// 预留的 buffer offset；当前 buffer 更新辅助函数固定使用 offset `0`。
            uint32_t offset_ = 0;

            /// buffer descriptor 的 range，单位为字节。
            VkDeviceSize bufferBytes = 0;
        };

        // void updateDescriptorSets(uint32_t set, uint32_t index,
        //                           const std::vector<SetBindings>& bindings);

        /// @brief 立即将一组 sampler descriptors 写入指定 descriptor set 实例。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] bindings 要写入的 sampler bindings。
        void updateSamplersDescriptorSets(uint32_t set, uint32_t index, const std::vector<SetBindings>& bindings);

        /// @brief 立即将一组 sampled-image descriptors 写入指定 descriptor set 实例。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] bindings 要写入的 texture bindings。
        void updateTexturesDescriptorSets(uint32_t set, uint32_t index, const std::vector<SetBindings>& bindings);

        /// @brief 立即将一组 buffer descriptors 写入指定 descriptor set 实例。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] type buffer descriptor 类型。
        /// @param[in] bindings 要写入的 buffer bindings。
        /// @note 当前实现固定使用 buffer offset `0`，range 来自 `SetBindings::bufferBytes`。
        void updateBuffersDescriptorSets(uint32_t set,
                                         uint32_t index,
                                         VkDescriptorType type,
                                         const std::vector<SetBindings>& bindings);

        /// @brief 提交并清空由 `bindResource()` 排队的 descriptor 写入及其临时后备数据。
        void updateDescriptorSets();

        /// @brief 为 buffer 或 texel-buffer descriptor 排队一次写入。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] binding 目标 binding 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] buffer 要写入的 buffer；本对象不会保留其 `std::shared_ptr`。
        /// @param[in] offset buffer descriptor 的字节偏移。
        /// @param[in] size buffer descriptor 的字节范围。
        /// @param[in] type descriptor 类型。
        /// @param[in] format texel-buffer 类型使用的格式；其他类型忽略。
        /// @pre 目标 descriptor set 已分配；资源必须存活到写入提交并覆盖其 GPU 使用期。
        void bindResource(uint32_t set,
                          uint32_t binding,
                          uint32_t index,
                          std::shared_ptr<Buffer> buffer,
                          uint32_t offset,
                          uint32_t size,
                          VkDescriptorType type,
                          VkFormat format = VK_FORMAT_UNDEFINED);

        /// @brief 为一组 texture 排队 sampled-image 或 combined-image-sampler descriptor 写入。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] binding 目标 binding 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] textures 要写入的 texture 列表。
        /// @param[in] sampler 可选 sampler；提供时同一个 sampler 应用于所有非空 texture。
        /// @param[in] dstArrayElement descriptor array 的起始元素。
        /// @note 空 texture 元素会被跳过；本对象不会保留传入资源的 `std::shared_ptr`。
        void bindResource(uint32_t set,
                          uint32_t binding,
                          uint32_t index,
                          std::span<std::shared_ptr<Texture>> textures,
                          std::shared_ptr<Sampler> sampler = nullptr,
                          uint32_t dstArrayElement = 0);

        /// @brief 为一组 sampler 排队 descriptor 写入。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] binding 目标 binding 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] samplers 要写入的 sampler 列表。
        void bindResource(uint32_t set, uint32_t binding, uint32_t index, std::span<std::shared_ptr<Sampler>> samplers);

        /// @brief 为一组 image view 排队使用 `VK_IMAGE_LAYOUT_GENERAL` 的 descriptor 写入。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] binding 目标 binding 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] imageViews 要写入的 image view 句柄列表。
        /// @param[in] type image descriptor 类型。
        /// @note 本对象只缓存底层句柄，不保留传入的 `std::shared_ptr`。
        void bindResource(uint32_t set,
                          uint32_t binding,
                          uint32_t index,
                          std::span<std::shared_ptr<VkImageView>> imageViews,
                          VkDescriptorType type);

        /// @brief 为一组完整 buffer 排队 descriptor array 写入。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] binding 目标 binding 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] buffers 要写入的 buffer 列表。
        /// @param[in] type buffer descriptor 类型。
        /// @note 每个 descriptor 使用 offset `0` 和对应 `Buffer::size()` 作为 range。
        void bindResource(uint32_t set,
                          uint32_t binding,
                          uint32_t index,
                          std::vector<std::shared_ptr<Buffer>> buffers,
                          VkDescriptorType type);

        /// @brief 为单个 texture 排队使用 `VK_IMAGE_LAYOUT_GENERAL` 的 descriptor 写入。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] binding 目标 binding 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] texture 要写入的 texture。
        /// @param[in] type image descriptor 类型。
        void bindResource(uint32_t set,
                          uint32_t binding,
                          uint32_t index,
                          std::shared_ptr<Texture> texture,
                          VkDescriptorType type);

        /// @brief 为单个 texture 与 sampler 排队 combined descriptor 写入。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] binding 目标 binding 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] texture 要写入的 texture。
        /// @param[in] sampler 要与 texture 配对的 sampler。
        /// @param[in] type image descriptor 类型。
        /// @note 当前实现使用 `VK_IMAGE_LAYOUT_GENERAL`。
        void bindResource(uint32_t set,
                          uint32_t binding,
                          uint32_t index,
                          std::shared_ptr<Texture> texture,
                          std::shared_ptr<Sampler> sampler,
                          VkDescriptorType type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

        /// @brief 为 acceleration structure 排队 descriptor 写入。
        /// @param[in] set 目标 Vulkan set 编号。
        /// @param[in] binding 目标 binding 编号。
        /// @param[in] index 已分配 descriptor set 的实例索引。
        /// @param[in] accelStructHandle acceleration structure 句柄的地址。
        /// @pre @p accelStructHandle 指向的句柄存储必须至少保持有效到 `updateDescriptorSets()` 提交写入。
        void bindResource(uint32_t set,
                          uint32_t binding,
                          uint32_t index,
                          VkAccelerationStructureKHR* accelStructHandle);
    private:
        /// @brief 根据 `graphicsPipelineDesc_` 创建图形 pipeline 及 layout。
        void createGraphicsPipeline();

        /// @brief 根据 `computePipelineDesc_` 创建计算 pipeline 及 layout。
        void createComputePipeline();

        /// @brief 根据 `rayTracingPipelineDesc_` 创建光线追踪 pipeline 及 layout。
        void createRayTracingPipeline();

        /// @brief 创建由 descriptor set layouts 和 push constant ranges 组成的 pipeline layout。
        VkPipelineLayout createPipelineLayout(const std::vector<VkDescriptorSetLayout>& descLayouts,
                                              const std::vector<VkPushConstantRange>& pushConsts) const;

        /// @brief 根据当前 pipeline 类型惰性创建 descriptor pool。
        void initDescriptorPool();

        /// @brief 根据当前 pipeline 类型创建并缓存 descriptor set layouts。
        void initDescriptorLayout();
    private:
        /// 非拥有指针；`Context` 及其逻辑设备必须比本对象存活更久。
        const Context* context_ = nullptr;

        /// 用于 Vulkan 对象调试名称的标签。
        std::string name_;

        /// 图形 pipeline 描述的副本；仅图形构造路径使用。
        GraphicsPipelineDescriptor graphicsPipelineDesc_;

        /// 计算 pipeline 描述的副本；仅计算构造路径使用。
        ComputePipelineDescriptor computePipelineDesc_;

        /// 光线追踪 pipeline 描述的副本；仅光线追踪构造路径使用。
        RayTracingPipelineDescriptor rayTracingPipelineDesc_;

        /// 当前 pipeline 类型对应的 Vulkan bind point。
        VkPipelineBindPoint bindPoint_ = VK_PIPELINE_BIND_POINT_GRAPHICS;

        /// 由本对象创建并销毁的 Vulkan pipeline。
        VkPipeline vkPipeline_ = VK_NULL_HANDLE;

        /// 由本对象创建并销毁的 Vulkan pipeline layout。
        VkPipelineLayout vkPipelineLayout_ = VK_NULL_HANDLE;

        /// 图形 pipeline 创建时使用的非拥有 render pass 句柄。
        VkRenderPass vkRenderPass_ = VK_NULL_HANDLE;

        /// @brief 保存某个 set layout 及从该 layout 分配的 descriptor sets。
        struct DescriptorSet
        {
            /// 由 `vkDescriptorPool_` 管理的 descriptor set 句柄。
            std::vector<VkDescriptorSet> vkSets_;

            /// 由本对象创建并销毁的 descriptor set layout。
            VkDescriptorSetLayout vkLayout_ = VK_NULL_HANDLE;
        };

        /// 以 Vulkan set 编号为键保存 layout 和已分配实例。
        std::unordered_map<uint32_t, DescriptorSet> descriptorSets_;

        /// 首次分配 descriptor sets 时创建，并由本对象销毁。
        VkDescriptorPool vkDescriptorPool_ = VK_NULL_HANDLE;

        /// 当前实现尚未使用的 push constant range 缓存。
        std::vector<VkPushConstantRange> pushConsts_;

        /// 为排队的 buffer descriptor 写入稳定保存后备数据。
        std::list<std::vector<VkDescriptorBufferInfo>> bufferInfo_;

        /// 为排队的 texel-buffer descriptor 写入稳定保存 buffer views。
        std::list<VkBufferView> bufferViewInfo_;

        /// 为排队的 image descriptor 写入稳定保存后备数据。
        std::list<std::vector<VkDescriptorImageInfo>> imageInfo_;

        /// 排队的 acceleration-structure descriptor 扩展结构。
        std::vector<VkWriteDescriptorSetAccelerationStructureKHR> accelerationStructInfo_;

        /// 等待下一次 `updateDescriptorSets()` 提交的 descriptor 写入。
        std::vector<VkWriteDescriptorSet> writeDescSets_;

        /// 仅用于部分延迟 descriptor 更新路径的互斥锁。
        std::mutex mutex_;
    };

}  // namespace Huli::Vulkan
