#ifndef RKPLATFORM_DEVICE_INFERENCE_DEVICE_H
#define RKPLATFORM_DEVICE_INFERENCE_DEVICE_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace rkplatform::device {

enum class TensorDataType {
    kUnknown,
    kFloat32,
    kFloat16,
    kInt8,
    kUInt8,
    kInt16,
    kUInt16,
    kInt32,
    kUInt32,
    kInt64,
    kBool,
};

enum class TensorLayout {
    kUnknown,
    kNchw,
    kNhwc,
};

struct TensorInfo {
    std::vector<std::uint32_t> shape;
    TensorDataType data_type = TensorDataType::kUnknown;
    TensorLayout layout = TensorLayout::kUnknown;
};

struct InferenceOutput {
    std::vector<std::uint32_t> shape;
    std::vector<float> data;
};

class InferenceDevice final {
public:
    // 创建未初始化的推理设备。
    InferenceDevice();

    // 释放推理设备资源。
    ~InferenceDevice();

    // 禁止复制推理设备。
    InferenceDevice(const InferenceDevice&) = delete;
    // 禁止复制赋值推理设备。
    InferenceDevice& operator=(const InferenceDevice&) = delete;

    // 加载并初始化推理模型。
    bool Initialize(const std::string& model_path);

    // 执行单输入模型推理。
    bool Infer(
        std::vector<InferenceOutput>& output,
        const void* input_data,
        std::size_t input_size,
        TensorDataType input_type,
        TensorLayout input_layout,
        bool pass_through = false
    );

    // 获取模型输入张量信息。
    const TensorInfo& GetInputInfo() const noexcept;

    // 获取模型输出张量信息。
    const std::vector<TensorInfo>& GetOutputInfos() const noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace rkplatform::device

#endif  // RKPLATFORM_DEVICE_INFERENCE_DEVICE_H
