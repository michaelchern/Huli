#pragma once

#ifdef _WIN32
#include <glslang/Public/ShaderLang.h>
#endif

#include <string>
#include <vector>

#include "Common.hpp"
#include "Utility.hpp"

/// @file ShaderModule.hpp
/// @brief 声明 Vulkan shader module 及其 GLSL/SPIR-V 加载流程的 RAII 封装。

namespace Huli::Vulkan
{

    class Context;

    /// @brief 管理 `VkShaderModule`，并记录 pipeline stage 与入口点元数据。
    ///
    /// `ShaderModule` 可以从文件或内存中的 SPIR-V 字节码创建 Vulkan shader module。
    /// Windows 构建还可以通过 glslang 预处理并编译 GLSL 源文件；着色器阶段由文件扩展名推断。
    ///
    /// @note `Context` 以非拥有指针形式借用，必须比本对象存活更久。
    /// @note `VkShaderStageFlagBits` 和入口点只作为后续创建 pipeline stage 时使用的元数据；
    ///       `vkCreateShaderModule` 本身不会验证入口点是否存在或 stage 是否匹配。
    /// @warning 当前类没有禁用复制操作；复制会让多个对象持有同一个 `VkShaderModule`，
    ///          从而导致重复销毁，因此不要复制 `ShaderModule` 对象。
    class ShaderModule final
    {
    public:
        /// @brief 从 shader 文件创建 Vulkan shader module，并使用指定入口点。
        /// @param[in] context Vulkan 上下文的非拥有指针，不能为空且必须比本对象存活更久。
        /// @param[in] filePath shader 文件路径；`.spv` 文件按二进制 SPIR-V 读取。
        /// @param[in] entryPoint shader 入口点名称。
        /// @param[in] stages 创建 pipeline stage 时使用的 Vulkan shader stage。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre `.spv` 文件必须包含有效 SPIR-V；Windows 上的 GLSL 文件扩展名必须属于实现支持的阶段。
        /// @pre GLSL 文件扩展名推断出的阶段、@p stages 和 @p entryPoint 必须与 shader 内容一致。
        /// @throws std::runtime_error 无法读取 @p filePath 时由 `Util::readFile()` 抛出。
        /// @warning 非 Windows 构建当前只支持 `.spv` 路径；其他文件不会进入 GLSL 编译流程。
        explicit ShaderModule(const Context* context,
                              const std::string& filePath,
                              const std::string& entryPoint,
                              VkShaderStageFlagBits stages,
                              const std::string& name);

        /// @brief 从内存中的 SPIR-V 字节码创建 Vulkan shader module。
        /// @param[in] context Vulkan 上下文的非拥有指针，不能为空且必须比本对象存活更久。
        /// @param[in] data 完整的 SPIR-V 二进制字节序列。
        /// @param[in] entryPoint shader 入口点名称。
        /// @param[in] stages 创建 pipeline stage 时使用的 Vulkan shader stage。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @pre @p data 必须包含有效 SPIR-V，字节数必须是 `sizeof(uint32_t)` 的整数倍。
        explicit ShaderModule(const Context* context,
                              const std::vector<char>& data,
                              const std::string& entryPoint,
                              VkShaderStageFlagBits stages,
                              const std::string& name);

        /// @brief 从 shader 文件创建 Vulkan shader module，并使用默认入口点 `main`。
        /// @param[in] context Vulkan 上下文的非拥有指针，不能为空且必须比本对象存活更久。
        /// @param[in] filePath shader 文件路径；`.spv` 文件按二进制 SPIR-V 读取。
        /// @param[in] stages 创建 pipeline stage 时使用的 Vulkan shader stage。
        /// @param[in] name 可选的 Vulkan 调试名称。
        /// @throws std::runtime_error 无法读取 @p filePath 时由 `Util::readFile()` 抛出。
        explicit ShaderModule(const Context* context,
                              const std::string& filePath,
                              VkShaderStageFlagBits stages,
                              const std::string& name);

        /// @brief 销毁本对象拥有的 Vulkan shader module。
        /// @pre `Context` 及其逻辑设备必须仍然有效。
        ~ShaderModule();

        /// @brief 获取底层 Vulkan shader module 句柄。
        /// @return 本对象拥有的 `VkShaderModule`；调用方不得销毁该句柄。
        VkShaderModule vkShaderModule() const;

        /// @brief 获取创建 pipeline stage 时使用的 shader stage。
        /// @return 构造时记录的 `VkShaderStageFlagBits`。
        VkShaderStageFlagBits vkShaderStageFlags() const;

        /// @brief 获取 shader 入口点。
        /// @return 构造时记录的入口点字符串引用；仅在本对象存活期间有效。
        const std::string& entryPoint() const;
    private:
#ifdef _WIN32
        /// @brief 根据 shader 文件扩展名推断 glslang shader stage。
        /// @param[in] fileName shader 文件名或路径，不能为空。
        /// @return 与扩展名对应的 `EShLanguage`。
        /// @pre 扩展名必须是 `.vert`、`.frag`、`.comp`、`.rgen`、`.rmiss`、`.rchit` 或 `.rahit`。
        EShLanguage shaderStageFromFileName(const char* fileName);

        /// @brief 使用 glslang 预处理并编译 GLSL 源码。
        /// @param[in] data 以空字符结尾的 GLSL 源码字节序列。
        /// @param[in] shaderStage glslang shader stage。
        /// @param[in] shaderDir 解析本地 `#include` 时使用的目录。
        /// @param[in] entryPoint shader 入口点名称，不能为空。
        /// @return 编译得到的 SPIR-V 二进制字节序列。
        /// @note 预处理或解析失败时会打印带行号源码和 glslang 日志，然后触发 `ASSERT`。
        std::vector<char> glslToSpirv(const std::vector<char>& data,
                                      EShLanguage shaderStage,
                                      const std::string& shaderDir,
                                      const char* entryPoint);
#endif

        /// @brief 将 shader 源码按行号输出到标准输出，用于编译错误诊断。
        void printShader(const std::vector<char>& data);

        /// @brief 读取 shader 文件，按需编译 GLSL，并创建 Vulkan shader module。
        void createShader(const std::string& filePath, const std::string& entryPoint, const std::string& name);

        /// @brief 根据 SPIR-V 字节码创建并命名 Vulkan shader module。
        void createShader(const std::vector<char>& spirv, const std::string& entryPoint, const std::string& name);
    private:
        /// 非拥有指针；`Context` 必须比本对象存活更久。
        const Context* context_ = nullptr;

        /// 由本对象创建并销毁的 Vulkan shader module。
        VkShaderModule vkShaderModule_ = VK_NULL_HANDLE;

        /// 构造时记录的 pipeline shader stage。
        VkShaderStageFlagBits vkStageFlags_;

        /// 构造时记录的 shader 入口点。
        std::string entryPoint_;
    };

}  // namespace Huli::Vulkan
