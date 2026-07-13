#pragma once

#include <array>
#include <glm/glm.hpp>
#include <glm/gtx/type_aligned.hpp>

/// @file Camera.hpp
/// @brief 声明变换矩阵集合以及位置、朝向、视锥和 temporal jitter 相机。

/// @brief 聚合渲染过程中常用的当前帧与上一帧变换矩阵。
/// @warning model、view 和 projection 没有成员初始化器，使用前必须由调用方写入有效矩阵。
struct UniformTransforms
{
    /// 模型空间到世界空间的变换矩阵。
    glm::aligned_mat4 model;

    /// 世界空间到观察空间的变换矩阵。
    glm::aligned_mat4 view;

    /// 观察空间到裁剪空间的投影矩阵。
    glm::aligned_mat4 projection;

    /// 上一帧的观察矩阵，默认初始化为单位矩阵。
    glm::aligned_mat4 prevViewMat = glm::mat4(1.0);

    /// 当前帧的 jitter 矩阵，默认初始化为单位矩阵。
    glm::aligned_mat4 jitter = glm::mat4(1.0);
};

namespace Huli::Render
{

    /// @brief 管理相机位置、观察旋转、固定透视投影、视锥平面和 temporal jitter。
    ///
    /// 本类是纯值类型，只拥有自身的向量、四元数和矩阵，不持有窗口、GPU 或其他外部资源。
    /// 投影参数在构造时生成 projectMatrix_，当前没有运行时修改 FOV、宽高比或裁剪面的接口。
    ///
    /// @note dirty 状态只跟踪位置、up vector 和 orientation 的修改；updateJitterMat() 不会标记 dirty。
    /// @note 本类没有内部同步，不保证同一实例可被多个线程并发读写。
    /// @warning target_ 仅在构造时参与初始 orientation 计算；当前移动和旋转操作不会更新或再次读取它。
    class Camera final
    {
    public:
        /// @brief 根据位置、观察目标和 up vector 创建固定 60 度垂直 FOV 的透视相机。
        /// @param[in] position 相机的初始世界空间位置。
        /// @param[in] target 初始观察目标，仅用于构造初始 orientation。
        /// @param[in] up 构造初始观察旋转时使用的 up vector。
        /// @param[in] near 近裁剪面距离。
        /// @param[in] far 远裁剪面距离。
        /// @param[in] aspect 投影视口的宽高比。
        /// @pre @p near 和 @p far 必须为正数且 @p near 小于 @p far，@p aspect 必须大于 `0`。
        /// @pre @p position 与 @p target 必须不同，@p up 必须非零且不能与观察方向共线。
        explicit Camera(const glm::vec3& position = glm::vec3(-9.f, 2.f, 2.f),
                        const glm::vec3& target = glm::vec3(0.0f, 0.0f, 0.0f),
                        const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
                        float near = .1f,
                        float far = 4000.f,
                        float aspect = 800.f / 600.f);

        /// @brief 根据当前相机基向量和投影参数计算六个世界空间视锥平面。
        /// @return 依次为 left、down、right、top、front、back 的平面；每个 `vec4` 表示
        ///         `dot(normal, point) + w = 0`。
        /// @note 此函数使用固定 fov_、nearP_、farP_ 和 aspect_，不包含 jitterMat_ 的偏移。
        std::array<glm::vec4, 6> calculateFrustumPlanes();

        /// @brief 沿调用方提供的方向移动相机并标记 dirty。
        /// @param[in] direction 世界空间移动方向；当前实现不会归一化该向量。
        /// @param[in] increment 移动增量，最终位移为 `direction * increment * kSpeedKey`。
        /// @note 此函数不会更新 target_。
        void move(const glm::vec3& direction, float increment);

        /// @brief 设置相机世界空间位置并标记 dirty。
        /// @param[in] position 新的位置。
        /// @note 此函数不会更新 target_ 或 orientation_。
        void setPos(const glm::vec3& position);

        /// @brief 归一化并保存旋转约束使用的 up vector，同时标记 dirty。
        /// @param[in] up 新的非零 up vector。
        /// @warning 当前实现不会立即重算 orientation_；新 up vector 只会在后续 rotate() 重建观察旋转时生效。
        void setUpVector(const glm::vec3& up);

        /// @brief 根据二维输入增量更新 orientation，并使用保存的 up_ 重新约束观察旋转。
        /// @param[in] delta 水平和垂直旋转输入；当前实现将其分别映射到 Y 轴和 X 轴旋转。
        /// @param[in] deltaT 输入缩放系数，默认使用 kSpeed；其名称不代表函数会自行读取时间。
        /// @note 旋转会归一化四元数、标记 dirty，并断言最终 orientation 不包含 NaN。
        void rotate(const glm::vec2& delta, double deltaT = kSpeed);

        /// @brief 获取构造时计算的固定透视投影矩阵。
        /// @return 不包含 temporal jitter 的 projectMatrix_ 副本。
        glm::mat4 getProjectMatrix() const;

        /// @brief 根据当前 orientation 和 position 计算观察矩阵。
        /// @return `mat4_cast(orientation_) * translate(-position_)`。
        glm::mat4 viewMatrix() const;

        /// @brief 获取 orientation 矩阵第三行的 xyz 分量。
        /// @return `(view[0][2], view[1][2], view[2][2])`；当前 rotate() 将其相反方向作为观察前向。
        glm::vec3 direction() const;

        /// @brief 获取当前 orientation 的 Euler angles。
        /// @return 以度为单位的 XYZ Euler angles。
        glm::vec3 eulerAngles() const;

        /// @brief 使用以度为单位的 Euler angles 直接重建 orientation 并标记 dirty。
        /// @param[in] dir XYZ Euler angles，单位为度。
        /// @note 此函数不会更新 up_ 或 target_。
        void setEulerAngles(const glm::vec3& dir);

        /// @brief 获取 orientation 矩阵第一行的 xyz 分量。
        glm::vec3 right() const;

        /// @brief 获取由 right() 与 direction() 叉积计算并归一化的相机 up vector。
        /// @note 返回值来自 orientation_，不直接返回构造或 setUpVector() 保存的 up_。
        glm::vec3 up() const;

        /// @brief 获取相机的世界空间位置。
        glm::vec3 position() const;

        /// @brief 查询相机 pose 是否自上次 setNotDirty() 后发生修改。
        /// @return pose 被修改时返回 `true`；jitter 更新不会影响该状态。
        bool isDirty() const;

        /// @brief 清除相机 pose 的 dirty 标记。
        void setNotDirty();

        /// @brief 使用 base 2/3 的 Van der Corput 序列更新 temporal jitter。
        /// @param[in] frameIndex 当前帧下标，用于在 @p numSamples 个样本之间循环。
        /// @param[in] numSamples jitter 序列的循环样本数量。
        /// @param[in] width 渲染目标宽度，用于将亚像素偏移换算到 UV/NDC。
        /// @param[in] height 渲染目标高度，用于将亚像素偏移换算到 UV/NDC。
        /// @pre @p numSamples、@p width 和 @p height 都必须大于 `0`。
        /// @note 原始偏移保存在 jitterVal_，范围约为 `[-0.5, 0.5)`；其两倍 UV 偏移写入
        ///       jitterMat_[2][0] 和 jitterMat_[2][1]。
        /// @note 此函数不会修改 isDirty_。
        void updateJitterMat(uint32_t frameIndex, int numSamples, int width, int height);

        /// @brief 获取当前 temporal jitter 矩阵。
        glm::mat4 jitterMat() const { return jitterMat_; }

        /// @brief 获取最近一次生成的亚像素 jitter 偏移。
        /// @pre 必须先调用 updateJitterMat()；jitterVal_ 当前没有显式初始值。
        glm::vec2 jitterInPixelSpace() const { return jitterVal_; }

        /// @brief 获取最近一次生成的 NDC jitter 偏移。
        /// @return jitterMat_ 索引为 2 的列（第三列）的 x、y 分量。
        glm::vec2 jitterInNDCSpace() const { return glm::vec2(jitterMat_[2][0], jitterMat_[2][1]); }
    private:
        /// 相机的世界空间位置。
        glm::vec3 position_;

        /// 构造和 rotate() 约束观察旋转时使用的 up vector。
        glm::vec3 up_;

        /// 初始观察目标；当前实现仅在构造时使用。
        glm::vec3 target_;

        /// 表示观察旋转的四元数。
        glm::quat orientation_;

        /// 构造时根据固定 FOV 和裁剪参数生成的透视投影矩阵。
        glm::mat4 projectMatrix_{1.0f};

        /// temporal jitter 矩阵，仅修改索引为 2 的列（第三列）的 x、y 分量。
        glm::mat4 jitterMat_{1.0f};

        /// 最近一次生成的亚像素 jitter 偏移；首次更新前没有显式初始化。
        glm::vec2 jitterVal_;

        /// 近裁剪面距离。
        float nearP_ = 10.f;

        /// 远裁剪面距离。
        float farP_ = 4000.0f;

        /// 固定的垂直 FOV，单位为度。
        float fov_ = 60.0f;

        /// 投影视口的宽高比。
        float aspect_ = 800.0 / 600.0;

        /// 相机 pose 是否发生修改；不跟踪 jitter 更新。
        bool isDirty_ = true;

        /// rotate() 的默认输入缩放系数。
        static constexpr float kSpeed = 4.0f;

        /// move() 的固定移动缩放系数。
        static constexpr float kSpeedKey = 0.3f;
    };

}  // namespace Huli::Render
