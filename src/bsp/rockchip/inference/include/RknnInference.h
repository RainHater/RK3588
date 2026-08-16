#ifndef RKNN_INFERENCE_H
#define RKNN_INFERENCE_H

#include <rknpu2/rknn_api.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Logger.h"

namespace rkplatform::bsp {

// 单个模型输出张量
struct RknnOutput {
    std::vector<std::uint32_t> shape;  // 输出形状
    std::vector<float> data;           // 浮点输出数据
};

class RknnInference {
public:
    // 创建未初始化的 RKNN 推理适配器。
    explicit RknnInference();

    // 释放 RKNN 上下文。
    ~RknnInference();

    // 禁止复制 RKNN 上下文。
    RknnInference(const RknnInference&) = delete;
    // 禁止复制赋值 RKNN 上下文。
    RknnInference& operator=(const RknnInference&) = delete;

    // 加载并初始化 RKNN 模型。
    bool Initialize(const std::string& model_path);

    // 执行单输入 RKNN 模型推理。
    bool Infer(
        std::vector<RknnOutput>& output,
        const void* input_data,
        std::size_t input_size,
        rknn_tensor_type input_type,
        rknn_tensor_format input_format,
        bool pass_through = false
    );

    // 获取模型输入张量信息。
    const rknn_tensor_attr& GetInputAttr() const noexcept;

    // 获取模型输出张量信息。
    const std::vector<rknn_tensor_attr>& GetOutputAttrs() const noexcept;

private:
    // 从文件加载 RKNN 模型。
    bool LoadModel(const std::string& model_path);

    // 查询模型输入输出信息。
    bool QueryModelInfo();

    // 检查 RKNN 接口返回值。
    bool Check(int ret, const char* operation);

private:
    rknn_context m_context = 0;
    std::vector<std::uint8_t> m_model_data;
    rknn_tensor_attr m_input_attr{};
    std::vector<rknn_tensor_attr> m_output_attrs;
    std::shared_ptr<spdlog::logger> m_logger;
};

}  // namespace rkplatform::bsp

#endif  // RKNN_INFERENCE_H
