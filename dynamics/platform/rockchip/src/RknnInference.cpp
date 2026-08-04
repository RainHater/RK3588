#include "RknnInference.h"

#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

RknnInference::RknnInference()
    : m_logger(LoggerWithTag::GetLogger("RknnInference"))
{

}

RknnInference::~RknnInference(){
    if (m_context != 0) {
        rknn_destroy(m_context);
        m_context = 0;
    }
}

bool RknnInference::Initialize(const std::string& model_path){
    try {
        LoadModel(model_path);
        QueryModelInfo();
    } catch (...) {
        if (m_context != 0) {
            rknn_destroy(m_context);
            m_context = 0;
        }

        return false;
    }

    return true;
}

bool RknnInference::Infer(
    std::vector<RknnOutput>& output,
    const void* input_data,
    std::size_t input_size,
    rknn_tensor_type input_type,
    rknn_tensor_format input_format,
    bool pass_through
)
{
    if (m_context == 0) {
        m_logger->error("RKNN模型尚未初始化");
        return false;
    }

    if (input_data == nullptr) {
        m_logger->error("模型输入数据不能为空");
        return false;
    }

    if (input_size == 0) {
        m_logger->error("模型输入数据大小不能为0");
        return false;
    }

    if (input_size > std::numeric_limits<std::uint32_t>::max()) {
        m_logger->error("模型输入数据过大");
        return false;
    }

    rknn_input input{};

    input.index = 0;
    input.buf = const_cast<void*>(input_data);
    input.size =
        static_cast<std::uint32_t>(input_size);
    input.type = input_type;
    input.fmt = input_format;
    input.pass_through =
        pass_through ? 1 : 0;

    bool res = Check(
        rknn_inputs_set(
            m_context,
            1,
            &input),
        "设置模型输入"
    );

    if (!res){
        return false;
    }

    res = Check(
        rknn_run(
            m_context,
            nullptr),
        "执行模型推理"
    );

    if (!res){
        return false;
    }

    std::vector<rknn_output> raw_outputs(
        m_output_attrs.size());

    for (std::size_t i = 0;
         i < raw_outputs.size();
         ++i) {
        raw_outputs[i] = {};
        raw_outputs[i].index =
            static_cast<std::uint32_t>(i);

        // 将量化输出转换成float
        raw_outputs[i].want_float = 1;

        // 由RKNN运行库分配输出内存
        raw_outputs[i].is_prealloc = 0;
    }

    res = Check(
        rknn_outputs_get(
            m_context,
            static_cast<std::uint32_t>(
                raw_outputs.size()),
            raw_outputs.data(),
            nullptr),
        "获取模型输出"
    );

    if (!res){
        return false;
    }

    // std::vector<RknnOutput> results;
    output.reserve(raw_outputs.size());

    try {
        for (std::size_t i = 0; i < raw_outputs.size(); i++) {
            if (raw_outputs[i].buf == nullptr) {
                m_logger->error("RKNN输出数据为空, 输出索引={}", std::to_string(i));
                return false;
            }

            RknnOutput result;

            const rknn_tensor_attr& attr =
                m_output_attrs[i];

            result.shape.assign(
                attr.dims,
                attr.dims + attr.n_dims);

            const auto* output_data =
                static_cast<const float*>(
                    raw_outputs[i].buf);

            const std::size_t output_count =
                raw_outputs[i].size /
                sizeof(float);

            result.data.assign(
                output_data,
                output_data + output_count);

            output.push_back(
                std::move(result));
        }
    } catch (...) {
        rknn_outputs_release(
            m_context,
            static_cast<std::uint32_t>(
                raw_outputs.size()),
            raw_outputs.data());

        return false;
    }

    res = Check(
        rknn_outputs_release(
            m_context,
            static_cast<std::uint32_t>(
                raw_outputs.size()),
            raw_outputs.data()),
        "释放模型输出"
    );

    return res;
}

const rknn_tensor_attr& RknnInference::GetInputAttr() const noexcept{
    return m_input_attr;
}

const std::vector<rknn_tensor_attr>& RknnInference::GetOutputAttrs() const noexcept{
    return m_output_attrs;
}

bool RknnInference::LoadModel(const std::string& model_path){
    std::ifstream file(model_path, std::ios::binary | std::ios::ate);

    if (!file) {
        m_logger->error("无法打开RKNN模型: {}", model_path);
        return false;
    }

    const std::streamsize model_size = file.tellg();

    if (model_size <= 0) {
        m_logger->error("RKNN模型文件为空: {}", model_path);
        return false;
    }

    if (static_cast<std::uint64_t>(model_size) > std::numeric_limits<std::uint32_t>::max()) {
        m_logger->error("RKNN模型文件过大: {}", model_path);
        return false;
    }

    m_model_data.resize(static_cast<std::size_t>(model_size));

    file.seekg(0, std::ios::beg);

    if (!file.read(reinterpret_cast<char*>(m_model_data.data()), model_size)) {
        m_logger->error("读取RKNN模型失败: {}", model_path);
        return false;
    }

    const int ret = rknn_init(
        &m_context,
        m_model_data.data(),
        static_cast<std::uint32_t>(m_model_data.size()),
        0,
        nullptr);

    return Check(ret, "初始化RKNN模型");
}

bool RknnInference::QueryModelInfo(){
    if (m_context == 0) {
        m_logger->error("RKNN上下文尚未初始化!");
        return false;
    }

    rknn_input_output_num io_num{};

    bool res = Check(
        rknn_query(
            m_context,
            RKNN_QUERY_IN_OUT_NUM,
            &io_num,
            sizeof(io_num)),
        "查询模型输入输出数量"
    );

    if (!res){
        return false;
    }

    if (io_num.n_input != 1) {
        m_logger->error("当前推理类只支持单输入模型, 实际输入数量={}", std::to_string(io_num.n_input));
        return false;
    }

    if (io_num.n_output == 0) {
        m_logger->error("RKNN模型没有输出张量");
        return false;
    }

    m_input_attr = {};
    m_input_attr.index = 0;

    res = Check(
        rknn_query(
            m_context,
            RKNN_QUERY_INPUT_ATTR,
            &m_input_attr,
            sizeof(m_input_attr)),
        "查询模型输入属性"
    );

    if (!res){
        return false;
    }

    m_output_attrs.clear();
    m_output_attrs.resize(io_num.n_output);

    for (std::uint32_t i = 0;
         i < io_num.n_output;
         ++i) {
        m_output_attrs[i] = {};
        m_output_attrs[i].index = i;

        res = Check(
            rknn_query(
                m_context,
                RKNN_QUERY_OUTPUT_ATTR,
                &m_output_attrs[i],
                sizeof(rknn_tensor_attr)),
            "查询模型输出属性"
        );
        if (!res){
            return false;
        }
    }
    return true;
}

bool RknnInference::Check(int ret, const char* operation){
    if (ret != RKNN_SUCC) {
        m_logger->error("{} 失败, ret={}", std::string(operation), std::to_string(ret));
        return false;
    }
    return true;
}
