#ifndef RKNN_INFERENCE_H
#define RKNN_INFERENCE_H

#include <rknpu2/rknn_api.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Logger.h"

// 单个模型输出张量
struct RknnOutput {
    std::vector<std::uint32_t> shape;  // 输出形状
    std::vector<float> data;           // 浮点输出数据
};

class RknnInference {
public:
    // 加载并初始化RKNN模型
    explicit RknnInference();

    // 释放RKNN上下文
    ~RknnInference();

    RknnInference(const RknnInference&) = delete;
    RknnInference& operator=(const RknnInference&) = delete;

    // 初始化
    bool Initialize(const std::string& model_path);

    /**
     * 执行单输入模型推理。
     *
     * input_data：已经完成尺寸、颜色、归一化等处理的输入数据
     * input_size：输入数据字节数
     * input_type：传入数据类型
     * input_format：传入数据布局
     * pass_through：是否直接传递RKNN原生数据
     */
    bool Infer(
        std::vector<RknnOutput>& output,
        const void* input_data,
        std::size_t input_size,
        rknn_tensor_type input_type,
        rknn_tensor_format input_format,
        bool pass_through = false
    );

    // 获取模型输入张量信息
    const rknn_tensor_attr& GetInputAttr() const noexcept;

    // 获取模型输出张量信息
    const std::vector<rknn_tensor_attr>& GetOutputAttrs() const noexcept;

private:
    // 加载RKNN模型
    bool LoadModel(const std::string& model_path);

    // 查询模型输入输出信息
    bool QueryModelInfo();

    // 检查RKNN接口返回值
    bool Check(int ret, const char* operation);

private:
    rknn_context m_context = 0;
    std::vector<std::uint8_t> m_model_data;
    rknn_tensor_attr m_input_attr{};
    std::vector<rknn_tensor_attr> m_output_attrs;
    std::shared_ptr<LoggerWithTag> m_logger;
};

#endif  // RKNN_INFERENCE_H
