#pragma once

#include <list>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "Common.hpp"
#include "Utility.hpp"

/// @file PhysicalDevice.hpp
/// @brief 声明物理设备能力查询、队列族选择和 Surface 支持信息的封装。

namespace Huli::Vulkan
{

    /// @brief 缓存 Vulkan 物理设备的能力、属性、扩展和队列族信息。
    ///
    /// 构造时查询物理设备的 feature、property、内存、队列族和设备扩展，并在提供
    /// `VkSurfaceKHR` 时额外缓存 Surface 格式、能力与呈现模式。reserveQueues() 随后从
    /// 已缓存的队列族中选择各类队列，供 `Context` 创建设备及取得队列句柄。
    ///
    /// @note 本类不拥有 `VkPhysicalDevice` 或 `VkSurfaceKHR`，不会销毁这些句柄；创建它们的
    ///       `VkInstance` 必须在本对象使用期间保持有效。
    /// @note 通过空 Surface 构造可用于离屏渲染，但此时没有有效的 Surface 能力快照。
    /// @note reserveQueues() 会修改内部选择结果，且本类没有内部同步；不要与其他访问并发调用。
    /// @warning 本类当前使用编译器生成的复制和移动操作，而 feature/property 结构中的 `pNext`
    ///          指向对象自身的成员。复制或移动后，不要遍历或复用这些 `pNext` 链进行 Vulkan 查询。
    class PhysicalDevice final
    {
    public:
        /// @brief 创建尚未关联 Vulkan 物理设备的占位对象。
        /// @note 在赋入由完整构造函数创建的对象前，不得调用依赖物理设备或查询结果的接口。
        explicit PhysicalDevice() {};

        /// @brief 查询并缓存指定 Vulkan 物理设备的能力和可用资源。
        /// @param[in] device 要查询的物理设备句柄，不转移所有权且不能为空。
        /// @param[in] surface 用于查询呈现支持的非拥有 Surface；离屏场景可传 `VK_NULL_HANDLE`。
        /// @param[in] requestedExtensions 调用方希望启用的设备扩展名称。
        /// @param[in] printEnumerations 是否将设备、扩展和 Surface 枚举结果输出到标准错误流。
        /// @param[in] enableRayTracing 是否在 feature 查询链中包含光线追踪相关结构。
        /// @pre @p device 必须来自仍然有效的 `VkInstance`。
        /// @pre 非空的 @p surface 必须与创建 @p device 的 `VkInstance` 兼容。
        /// @note 实际启用扩展集合仅取设备可用扩展与 @p requestedExtensions 的交集；缺失项不会在此处报错。
        explicit PhysicalDevice(VkPhysicalDevice device,
                                VkSurfaceKHR surface,
                                const std::vector<std::string>& requestedExtensions,
                                bool printEnumerations = false,
                                bool enableRayTracing = false);

        /// @brief 获取底层物理设备句柄。
        /// @return 本对象引用但不拥有的 `VkPhysicalDevice`；调用方不得销毁该句柄。
        [[nodiscard]] VkPhysicalDevice vkPhysicalDevice() const;

        /// @brief 获取物理设备报告的全部设备扩展名称。
        /// @return 构造时枚举得到的扩展名称列表，与请求启用的扩展集合不同。
        [[nodiscard]] const std::vector<std::string>& extensions() const;

        /// @brief 为请求的操作类型选择队列族，并在需要时选择呈现队列族。
        /// @param[in] requestedQueueTypes 非空的 `VkQueueFlags`，表示要选择的队列能力。
        /// @param[in] surface 用于检查呈现支持的非拥有 Surface；离屏渲染可传 `VK_NULL_HANDLE`。
        /// @pre 构造函数必须已经缓存有效的物理设备队列族信息。
        /// @note 当前算法按队列族顺序为 graphics、compute、transfer 和 sparse 各选择第一个匹配项，
        ///       并通过 `continue` 避免这些用途共享同一次匹配；呈现队列允许与它们共享队列族。
        /// @note 选择结果不会在调用开始时清空，多次调用会保留已经选择的队列族。
        /// @warning 当设备仅有一个同时支持多种操作的队列族时，当前独占选择策略可能无法为所有请求
        ///          的操作类型分别保留队列族。
        void reserveQueues(VkQueueFlags requestedQueueTypes, VkSurfaceKHR surface);

        /// @brief 汇总已选择队列族的索引和可用队列数量。
        /// @return 去重并按索引排序的 `(queueFamilyIndex, queueCount)` 列表。
        /// @note `queueCount` 是物理设备为整个队列族报告的队列总数，不是调用方请求的数量。
        [[nodiscard]] std::vector<std::pair<uint32_t, uint32_t>> queueFamilyIndexAndCount() const;

        /// @brief 获取已选择的 graphics 队列族索引。
        /// @return 成功选择时返回索引，否则返回 `std::nullopt`。
        [[nodiscard]] std::optional<uint32_t> graphicsFamilyIndex() const;

        /// @brief 获取已选择的 compute 队列族索引。
        /// @return 成功选择时返回索引，否则返回 `std::nullopt`。
        [[nodiscard]] std::optional<uint32_t> computeFamilyIndex() const;

        /// @brief 获取已选择的 transfer 队列族索引。
        /// @return 成功选择时返回索引，否则返回 `std::nullopt`。
        [[nodiscard]] std::optional<uint32_t> transferFamilyIndex() const;

        /// @brief 获取已选择的 sparse-binding 队列族索引。
        /// @return 成功选择时返回索引，否则返回 `std::nullopt`。
        [[nodiscard]] std::optional<uint32_t> sparseFamilyIndex() const;

        /// @brief 获取已选择的 presentation 队列族索引。
        /// @return 提供有效 Surface 且找到呈现支持时返回索引，否则返回 `std::nullopt`。
        [[nodiscard]] std::optional<uint32_t> presentationFamilyIndex() const;

        /// @brief 获取已选择 graphics 队列族报告的队列总数。
        /// @return 未选择 graphics 队列族时返回 `0`。
        [[nodiscard]] uint32_t graphicsFamilyCount() const { return graphicsQueueCount_; }

        /// @brief 获取已选择 compute 队列族报告的队列总数。
        /// @return 未选择 compute 队列族时返回 `0`。
        [[nodiscard]] uint32_t computeFamilyCount() const { return computeQueueCount_; }

        /// @brief 获取已选择 transfer 队列族报告的队列总数。
        /// @return 未选择 transfer 队列族时返回 `0`。
        [[nodiscard]] uint32_t transferFamilyCount() const { return transferQueueCount_; }

        /// @brief 获取已选择 sparse-binding 队列族报告的队列总数。
        /// @return 未选择 sparse-binding 队列族时返回 `0`。
        [[nodiscard]] uint32_t sparseFamilyCount() const { return sparseQueueCount_; }

        /// @brief 获取已选择 presentation 队列族报告的队列总数。
        /// @return 未选择 presentation 队列族时返回 `0`。
        [[nodiscard]] uint32_t presentationFamilyCount() const { return presentationQueueCount_; }

        /// @brief 获取构造时查询的 Surface 能力快照。
        /// @return 与构造函数所传 Surface 对应的 `VkSurfaceCapabilitiesKHR`。
        /// @pre 构造时必须提供非空的 `VkSurfaceKHR`。
        /// @warning 离屏构造不会初始化有效的 Surface 能力，不得在该模式下调用此接口。
        const VkSurfaceCapabilitiesKHR& surfaceCapabilities() const;

        /// @brief 获取构造时查询的物理设备 feature 快照。
        /// @return 以 `VkPhysicalDeviceFeatures2` 为根的查询结果。
        /// @warning 对象发生复制或移动后，不要遍历返回结构的 `pNext` 链。
        const VkPhysicalDeviceFeatures2& features() const { return features_; }

        /// @brief 获取构造时查询的物理设备 property 快照。
        /// @return 以 `VkPhysicalDeviceProperties2` 为根的查询结果。
        /// @warning 对象发生复制或移动后，不要遍历返回结构的 `pNext` 链。
        const VkPhysicalDeviceProperties2& properties() const { return properties_; }

        /// @brief 获取设备可用扩展与请求扩展的交集。
        /// @return `Context` 当前会传给 `vkCreateDevice` 的扩展名称集合。
        /// @note 结果不保留请求顺序和重复项，也不表示全部请求都已得到满足。
        const std::unordered_set<std::string>& enabledExtensions() const { return enabledExtensions_; }

        /// @brief 检查当前 feature 查询结果是否同时支持 Huli 使用的光线追踪能力。
        /// @return acceleration structure、ray-tracing pipeline 和 ray query feature 均可用时返回 `true`。
        /// @note 此检查只读取 feature 位，不额外验证相关扩展是否包含在 enabledExtensions() 中。
        bool isRayTracingSupported() const
        {
            return (accelStructFeature_.accelerationStructure && rayTracingFeature_.rayTracingPipeline &&
                    rayQueryFeature_.rayQuery);
        }

        /// @brief 获取光线追踪管线属性快照。
        /// @return 构造时由 `vkGetPhysicalDeviceProperties2` 填充的属性副本。
        /// @warning 返回副本的 `pNext` 仍指向本对象内部的 property 链，不得在本对象失效后遍历。
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProperties() const
        {
            return rayTracingPipelineProperties_;
        }

        /// @brief 获取 fragment density map 属性快照。
        /// @return 构造时由 `vkGetPhysicalDeviceProperties2` 填充的属性。
        const VkPhysicalDeviceFragmentDensityMapPropertiesEXT& fragmentDensityMapProperties() const
        {
            return fragmentDensityMapProperties_;
        }

        /// @brief 获取 QCOM fragment density map offset 属性快照。
        /// @return 构造时由 `vkGetPhysicalDeviceProperties2` 填充的属性。
        const VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM& fragmentDensityMapOffsetProperties() const
        {
            return fragmentDensityMapOffsetProperties_;
        }

        /// @brief 获取构造时为 Surface 枚举的呈现模式。
        /// @return Surface 支持的 `VkPresentModeKHR` 列表；离屏构造时为空。
        const std::vector<VkPresentModeKHR>& presentModes() const { return presentModes_; }

        /// @brief 检查物理设备是否报告 multiview feature。
        /// @return feature 可用时返回 `true`。
        bool isMultiviewSupported() const { return multiviewFeature_.multiview; }

        /// @brief 检查物理设备是否报告 fragment density map feature。
        /// @return feature 可用时返回 `true`。
        bool isFragmentDensityMapSupported() const { return fragmentDensityMapFeature_.fragmentDensityMap == VK_TRUE; }

        /// @brief 检查物理设备是否报告 QCOM fragment density map offset feature。
        /// @return feature 可用时返回 `true`。
        bool isFragmentDensityMapOffsetSupported() const
        {
            return fragmentDensityMapOffsetFeature_.fragmentDensityMapOffset == VK_TRUE;
        }
    private:
        /// @brief 查询并缓存指定 Surface 支持的格式。
        void enumerateSurfaceFormats(VkSurfaceKHR surface);

        /// @brief 查询并缓存指定 Surface 的能力。
        void enumerateSurfaceCapabilities(VkSurfaceKHR surface);

        /// @brief 查询并缓存指定 Surface 支持的呈现模式。
        void enumeratePresentationModes(VkSurfaceKHR surface);
    private:
        /// 非拥有句柄；创建它的 `VkInstance` 必须比本对象存活更久。
        VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;

        /// 物理设备报告的全部设备扩展名称。
        std::vector<std::string> extensions_;

        /// property 查询链：fragment-density-map offset 属性节点。
        VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM fragmentDensityMapOffsetProperties_{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_QCOM,
            .pNext = nullptr,
        };

        /// property 查询链：fragment-density-map 属性节点。
        VkPhysicalDeviceFragmentDensityMapPropertiesEXT fragmentDensityMapProperties_{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_PROPERTIES_EXT,
            .pNext = &fragmentDensityMapOffsetProperties_,
        };

        /// property 查询链：ray-tracing pipeline 属性节点。
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties_{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR,
            .pNext = &fragmentDensityMapProperties_,
        };

        /// property 查询链根节点。
        VkPhysicalDeviceProperties2 properties_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
            .pNext = &rayTracingPipelineProperties_,
        };

        /// feature 查询链：fragment-density-map offset 节点。
        VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM fragmentDensityMapOffsetFeature_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM,
            .pNext = nullptr,
        };

        /// feature 查询链：fragment-density-map 节点。
        VkPhysicalDeviceFragmentDensityMapFeaturesEXT fragmentDensityMapFeature_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_FEATURES_EXT,
            .pNext = &fragmentDensityMapOffsetFeature_,
        };

        /// feature 查询链：multiview 节点。
        VkPhysicalDeviceMultiviewFeatures multiviewFeature_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_FEATURES,
            .pNext = &fragmentDensityMapFeature_,
        };

        /// feature 查询链：timeline semaphore 节点。
        VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeature_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
            .pNext = &multiviewFeature_,
        };

        /// feature 查询链：NV mesh shader 节点。
        VkPhysicalDeviceMeshShaderFeaturesNV meshShaderFeature_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV,
            .pNext = (void*)&timelineSemaphoreFeature_,
        };

        /// feature 查询链：ray query 节点。
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeature_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
            .pNext = (void*)&meshShaderFeature_,
        };

        /// feature 查询链：ray-tracing pipeline 节点。
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeature_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
            .pNext = (void*)&rayQueryFeature_,
        };

        /// feature 查询链：acceleration structure 节点。
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeature_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
            .pNext = (void*)&rayTracingFeature_,
        };

        /// feature 查询链：descriptor indexing 节点；禁用光线追踪查询时会跳过相关节点。
        VkPhysicalDeviceDescriptorIndexingFeatures descIndexFeature_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
            .pNext = (void*)&accelStructFeature_,
        };

        /// feature 查询链：buffer device address 节点。
        VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
            .pNext = (void*)&descIndexFeature_,
        };

        /// feature 查询链：Vulkan 1.2 节点。
        VkPhysicalDeviceVulkan12Features features12_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = (void*)&bufferDeviceAddressFeatures_,
        };

        /// feature 查询链根节点。
        VkPhysicalDeviceFeatures2 features_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = (void*)&features12_,
        };

        /// 构造时查询的物理设备内存属性；当前没有公开访问器。
        VkPhysicalDeviceMemoryProperties2 memoryProperties_ = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2,
        };

        /// 已选择的 graphics 队列族及其报告的队列总数。
        std::optional<uint32_t> graphicsFamilyIndex_;
        uint32_t graphicsQueueCount_ = 0;

        /// 已选择的 compute 队列族及其报告的队列总数。
        std::optional<uint32_t> computeFamilyIndex_;
        uint32_t computeQueueCount_ = 0;

        /// 已选择的 transfer 队列族及其报告的队列总数。
        std::optional<uint32_t> transferFamilyIndex_;
        uint32_t transferQueueCount_ = 0;

        /// 已选择的 sparse-binding 队列族及其报告的队列总数。
        std::optional<uint32_t> sparseFamilyIndex_;
        uint32_t sparseQueueCount_ = 0;

        /// 已选择的 presentation 队列族及其报告的队列总数。
        std::optional<uint32_t> presentationFamilyIndex_;
        uint32_t presentationQueueCount_ = 0;

        /// 物理设备报告的全部队列族属性，供 reserveQueues() 选择。
        std::vector<VkQueueFamilyProperties> queueFamilyProperties_;

        /// 构造时为非空 Surface 查询的格式列表；当前仅用于枚举输出。
        std::vector<VkSurfaceFormatKHR> surfaceFormats_;

        /// 构造时为非空 Surface 查询的能力快照。
        VkSurfaceCapabilitiesKHR surfaceCapabilities_;

        /// 构造时为非空 Surface 查询的呈现模式列表。
        std::vector<VkPresentModeKHR> presentModes_;

        /// 物理设备可用扩展与请求扩展的交集。
        std::unordered_set<std::string> enabledExtensions_;
    };

}  // namespace Huli::Vulkan
