#include "InferenceDevice.h"

#include "RknnInference.h"

#include <utility>

namespace rkplatform::device {
namespace {

TensorDataType ToDataType(rknn_tensor_type type) {
    switch (type) {
    case RKNN_TENSOR_FLOAT32:
        return TensorDataType::kFloat32;
    case RKNN_TENSOR_FLOAT16:
        return TensorDataType::kFloat16;
    case RKNN_TENSOR_INT8:
        return TensorDataType::kInt8;
    case RKNN_TENSOR_UINT8:
        return TensorDataType::kUInt8;
    case RKNN_TENSOR_INT16:
        return TensorDataType::kInt16;
    case RKNN_TENSOR_UINT16:
        return TensorDataType::kUInt16;
    case RKNN_TENSOR_INT32:
        return TensorDataType::kInt32;
    case RKNN_TENSOR_UINT32:
        return TensorDataType::kUInt32;
    case RKNN_TENSOR_INT64:
        return TensorDataType::kInt64;
    case RKNN_TENSOR_BOOL:
        return TensorDataType::kBool;
    default:
        return TensorDataType::kUnknown;
    }
}

bool ToRknnDataType(TensorDataType type, rknn_tensor_type& native_type) {
    switch (type) {
    case TensorDataType::kFloat32:
        native_type = RKNN_TENSOR_FLOAT32;
        return true;
    case TensorDataType::kFloat16:
        native_type = RKNN_TENSOR_FLOAT16;
        return true;
    case TensorDataType::kInt8:
        native_type = RKNN_TENSOR_INT8;
        return true;
    case TensorDataType::kUInt8:
        native_type = RKNN_TENSOR_UINT8;
        return true;
    case TensorDataType::kInt16:
        native_type = RKNN_TENSOR_INT16;
        return true;
    case TensorDataType::kUInt16:
        native_type = RKNN_TENSOR_UINT16;
        return true;
    case TensorDataType::kInt32:
        native_type = RKNN_TENSOR_INT32;
        return true;
    case TensorDataType::kUInt32:
        native_type = RKNN_TENSOR_UINT32;
        return true;
    case TensorDataType::kInt64:
        native_type = RKNN_TENSOR_INT64;
        return true;
    case TensorDataType::kBool:
        native_type = RKNN_TENSOR_BOOL;
        return true;
    case TensorDataType::kUnknown:
        return false;
    }

    return false;
}

TensorLayout ToLayout(rknn_tensor_format format) {
    switch (format) {
    case RKNN_TENSOR_NCHW:
        return TensorLayout::kNchw;
    case RKNN_TENSOR_NHWC:
        return TensorLayout::kNhwc;
    default:
        return TensorLayout::kUnknown;
    }
}

bool ToRknnLayout(TensorLayout layout, rknn_tensor_format& native_layout) {
    switch (layout) {
    case TensorLayout::kNchw:
        native_layout = RKNN_TENSOR_NCHW;
        return true;
    case TensorLayout::kNhwc:
        native_layout = RKNN_TENSOR_NHWC;
        return true;
    case TensorLayout::kUnknown:
        return false;
    }

    return false;
}

TensorInfo ToTensorInfo(const rknn_tensor_attr& attr) {
    TensorInfo info;
    info.shape.assign(attr.dims, attr.dims + attr.n_dims);
    info.data_type = ToDataType(attr.type);
    info.layout = ToLayout(attr.fmt);
    return info;
}

}  // namespace

struct InferenceDevice::Impl {
    bsp::RknnInference inference;
    TensorInfo input_info;
    std::vector<TensorInfo> output_infos;
};

InferenceDevice::InferenceDevice()
    : m_impl(std::make_unique<Impl>())
{}

InferenceDevice::~InferenceDevice() = default;

bool InferenceDevice::Initialize(const std::string& model_path) {
    m_impl->input_info = {};
    m_impl->output_infos.clear();

    if (!m_impl->inference.Initialize(model_path)) {
        return false;
    }

    m_impl->input_info = ToTensorInfo(m_impl->inference.GetInputAttr());
    const auto& native_outputs = m_impl->inference.GetOutputAttrs();
    m_impl->output_infos.reserve(native_outputs.size());
    for (const auto& output : native_outputs) {
        m_impl->output_infos.push_back(ToTensorInfo(output));
    }

    return true;
}

bool InferenceDevice::Infer(
    std::vector<InferenceOutput>& output,
    const void* input_data,
    std::size_t input_size,
    TensorDataType input_type,
    TensorLayout input_layout,
    bool pass_through
)
{
    output.clear();

    rknn_tensor_type native_type{};
    rknn_tensor_format native_layout{};
    if (!ToRknnDataType(input_type, native_type) ||
        !ToRknnLayout(input_layout, native_layout)) {
        return false;
    }

    std::vector<bsp::RknnOutput> native_outputs;
    if (!m_impl->inference.Infer(
            native_outputs,
            input_data,
            input_size,
            native_type,
            native_layout,
            pass_through)) {
        return false;
    }

    output.reserve(native_outputs.size());
    for (auto& native_output : native_outputs) {
        output.push_back({
            std::move(native_output.shape),
            std::move(native_output.data)
        });
    }

    return true;
}

const TensorInfo& InferenceDevice::GetInputInfo() const noexcept {
    return m_impl->input_info;
}

const std::vector<TensorInfo>& InferenceDevice::GetOutputInfos() const noexcept {
    return m_impl->output_infos;
}

}  // namespace rkplatform::device
