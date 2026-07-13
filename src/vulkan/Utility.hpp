#pragma once

#include <cassert>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

/// @file Utility.hpp
/// @brief 声明 Vulkan 模块共用的断言、对象移动约束、文件和哈希辅助工具。

/// @brief 断言 @p expr 成立，并保留用于说明失败原因的 @p message 参数。
/// @note 当前实现不会输出 @p message；禁用标准 `assert` 后也不会检查 @p expr。
#define ASSERT(expr, message)                                                                      \
    {                                                                                              \
        void(message);                                                                             \
        assert(expr);                                                                              \
    }

/// @brief 禁用类的复制操作，并为其生成默认移动操作。
/// @param CLASS_NAME 需要限制复制并允许移动的类名。
/// @note 使用此宏不代表默认移动操作一定满足资源所有权语义；资源类仍需自行确认
///       移动后源对象不会重复释放句柄。
#define MOVABLE_ONLY(CLASS_NAME)                                                                   \
    CLASS_NAME(const CLASS_NAME&) = delete;                                                        \
    CLASS_NAME& operator=(const CLASS_NAME&) = delete;                                             \
    CLASS_NAME(CLASS_NAME&&) noexcept = default;                                                   \
    CLASS_NAME& operator=(CLASS_NAME&&) noexcept = default;

namespace Huli::Vulkan::Util
{
    /// @brief 使用 FNV-1 算法计算一段字节数据的 32 位哈希值。
    /// @param[in] key 待计算数据的起始地址。
    /// @param[in] len 待计算数据的长度，单位为字节。
    /// @return 输入字节序列对应的 32 位哈希值。
    /// @pre 当 @p len 大于 `0` 时，@p key 必须指向至少 @p len 个可读字节。
    /// @note 算法来源：
    ///       https://stackoverflow.com/questions/11413860/best-string-hashing-function-for-short-filenames
    uint32_t fnv_hash(const void* key, int len);

    /// @brief 将整个文件读取到字节数组中。
    /// @param[in] filePath 要读取的文件路径。
    /// @param[in] isBinary 是否以二进制模式读取。
    /// @return 文件内容；文本模式会在末尾额外添加空字符，二进制模式保持原始字节数。
    /// @throws std::runtime_error 文件无法打开时抛出。
    std::vector<char> readFile(const std::string& filePath, bool isBinary = false);

    /// @brief 将字节数组写入文件。
    /// @param[in] filePath 目标文件路径。
    /// @param[in] fileContents 要写入的内容。
    /// @param[in] isBinary 是否以二进制模式写入。
    /// @note 二进制模式会追加内容，文本模式会覆盖文件。
    /// @pre 文本模式下，@p fileContents 必须包含以空字符结尾的字符串。
    void writeFile(const std::string& filePath, const std::vector<char>& fileContents,
                   bool isBinary = false);

    /// @brief 判断字符串是否以指定片段结尾。
    /// @param[in] s 要检查的空字符结尾字符串。
    /// @param[in] part 期望位于字符串末尾的空字符结尾片段。
    /// @return 匹配时返回非零值，否则返回 `0`。
    /// @pre @p s 和 @p part 均不能为空，且 @p part 必须能在 @p s 中找到。
    /// @warning 当前实现只检查 @p part 在 @p s 中首次出现的位置；若相同片段较早出现，
    ///          即使末尾也存在该片段，仍可能返回 `0`。
    int endsWith(const char* s, const char* part);

    /// @brief 筛选可用名称与请求名称的交集。
    /// @param[in] availableExtensions 当前可用的扩展名或层名称。
    /// @param[in] requestedExtensions 调用方请求的扩展名或层名称。
    /// @return 同时存在于两个输入集合中的名称；结果不保留顺序和重复项。
    /// @note 两个输入按值传递，函数内部排序不会修改调用方的数据。
    std::unordered_set<std::string> filterExtensions(std::vector<std::string> availableExtensions,
                                                     std::vector<std::string> requestedExtensions);

    /// @brief 按顺序将一个或多个值合并到已有哈希种子中。
    /// @tparam T 当前值的类型，必须可由 `std::hash<T>` 计算哈希。
    /// @tparam Rest 其余待合并值的类型。
    /// @param[in,out] seed 要更新的哈希种子。
    /// @param[in] v 当前待合并的值。
    /// @param[in] rest 其余待合并的值。
    template <typename T, typename... Rest>
    void hash_combine(std::size_t& seed, const T& v, const Rest&... rest)
    {
        seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        (hash_combine(seed, rest), ...);
    }

}  // namespace Huli::Vulkan::Util
