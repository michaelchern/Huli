#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>

/// @file FPSCounter.hpp
/// @brief 提供基于调用方时间戳和帧计数的轻量级 FPS 采样器。

namespace Huli::Render
{

    /// @brief 按固定时间窗口估算 FPS，并将结果保存在固定容量的环形样本数组中。
    ///
    /// 调用方负责提供以秒为单位的单调时间戳，并在每个已处理帧调用 incFrame()。update() 会在距离上次
    /// 采样超过一秒时计算平均 FPS，可选择输出到 `std::cerr`，同时将结果写入内部样本数组。
    ///
    /// 本类只拥有自己的计数值和样本数组，不持有外部计时器或渲染资源。
    ///
    /// @note 本类没有内部同步；同一个实例应由单一线程使用，或由调用方负责同步。
    /// @warning 样本容量必须大于 `0`，否则取模和数组访问无效。
    /// @warning Windows 头文件若在此之前定义了 `min` 宏，fpsSamples() 中的 `std::min` 会被宏展开并导致
    ///          编译失败；当前头文件不会自行解除该宏。
    class FPSCounter final
    {
    public:
        /// @brief 创建 FPS 计数器并初始化固定容量的样本数组。
        /// @param[in] now 初始时间戳，单位为秒；后续 update() 必须使用相同时间源。
        /// @param[in] numSamples 保留的 FPS 样本数量，未写入的槽初始化为 `0`。
        /// @pre `0 < numSamples <= UINT32_MAX`。
        FPSCounter(double now, std::size_t numSamples = 100)
            : previous_{now},
              numSamplesStore_{numSamples}
        {
            samples_.resize(numSamplesStore_, 0.0f);
        }

        /// @brief 设置是否禁止向标准错误流输出 FPS。
        /// @param[in] silent 为 `true` 时只采样而不输出；为 `false` 时每次采样都会输出。
        void setSilent(bool silent) { silent_ = silent; }

        /// @brief 根据当前时间和累计帧数更新 FPS 样本。
        /// @param[in] now 当前时间戳，单位为秒。
        /// @pre @p now 应与构造函数使用相同的单调时间源，且不得早于上一次有效采样时间。
        /// @note 只有经过时间严格大于一秒时才计算新样本；否则本函数不修改状态。
        /// @note 新样本以 `float` 保存，非静默模式下同时以 `FPS: <value>` 格式写入 `std::cerr`。
        void update(double now)
        {
            const auto delta = now - previous_;
            if (delta > 1)
            {
                const auto fps = static_cast<double>(frame_ - previousFrame_) / delta;
                if (!silent_)
                {
                    std::cerr << "FPS: " << fps << std::endl;
                }
                previousFrame_ = frame_;
                previous_ = now;

                samples_[sample_ % numSamplesStore_] = fps;
                ++sample_;
            }
        }

        /// @brief 获取按当前实现计算起点排列的固定长度 FPS 样本副本。
        /// @return 包含 numSamplesStore_ 个元素的新数组；尚未写入的槽保持为 `0`。
        /// @warning 当前 startFrame 使用无符号减法；当累计样本数达到容量后可能发生下溢，因此返回顺序不一定是
        ///          严格的时间顺序。
        std::vector<float> fpsSamples()
        {
            std::vector<float> retVal(numSamplesStore_, 0.0f);
            const std::size_t startFrame = sample_ % numSamplesStore_ - std::min(sample_, numSamplesStore_);
            for (std::uint32_t i = 0; i < numSamplesStore_; ++i)
            {
                retVal[i] = samples_[(startFrame + i) % numSamplesStore_];
            }
            return retVal;
        }

        /// @brief 获取环形数组中下一次写入位置当前保存的值。
        /// @return `samples_[sample_ % numSamplesStore_]`；采样后该位置通常不是刚写入的最新样本。
        float current() const { return samples_[sample_ % numSamplesStore_]; }

        /// @brief 获取最近一次写入的 FPS 样本。
        /// @return 环形数组中前一个写入位置的值。
        /// @pre 至少已经生成一个 FPS 样本；首次采样前的无符号索引回绕不表示有效测量值。
        float last() const { return samples_[(sample_ - 1) % numSamplesStore_]; }

        /// @brief 将累计帧数增加一。
        /// @note 调用方通常应在每个已完成帧调用一次；本函数不会自动触发 FPS 采样。
        void incFrame() { ++frame_; }
    private:
        /// 上一次生成 FPS 样本时的时间戳，单位为秒。
        double previous_ = 0.0;

        /// 调用方通过 incFrame() 累计的总帧数。
        std::size_t frame_ = 0;

        /// 上一次生成 FPS 样本时记录的累计帧数。
        std::size_t previousFrame_ = 0;

        /// 已生成的 FPS 样本总数，同时用于计算环形写入位置。
        std::size_t sample_ = 0;

        /// 是否禁止向 `std::cerr` 输出采样结果。
        bool silent_ = false;

        /// 构造时确定的样本数组容量。
        std::size_t numSamplesStore_ = 100;

        /// 固定容量的 FPS 环形样本数组。
        std::vector<float> samples_;
    };

}  // namespace Huli::Render
