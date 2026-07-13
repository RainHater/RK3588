#include "RkTools.h"

namespace RkTools {
bool GetInputSize(
    const rknn_tensor_attr& attr,
    int& width, 
    int& height,
    int& channels
)
{
    if (attr.n_dims != 4) {
        return false;
    }

    if (attr.fmt == RKNN_TENSOR_NHWC) {
        height = static_cast<int>(attr.dims[1]);
        width = static_cast<int>(attr.dims[2]);
        channels = static_cast<int>(attr.dims[3]);
        return true;
    }

    if (attr.fmt == RKNN_TENSOR_NCHW) {
        channels = static_cast<int>(attr.dims[1]);
        height = static_cast<int>(attr.dims[2]);
        width = static_cast<int>(attr.dims[3]);
        return true;
    }

    return false;
}


cv::Mat PrepareFaceImage(const cv::Mat& bgr_image, int width, int height){
    cv::Mat rgb_image;
    cv::cvtColor(bgr_image, rgb_image, cv::COLOR_BGR2RGB);

    cv::Mat resized_image;
    cv::resize(rgb_image, resized_image, cv::Size(width, height), 0.0, 0.0,
               cv::INTER_LINEAR);

    return resized_image.isContinuous() ? resized_image : resized_image.clone();
}

void NormalizeFeature(std::vector<float>& feature){
    float square_sum = 0.0F;

    for (float value : feature) {
        square_sum += value * value;
    }

    const float norm = std::sqrt(square_sum);

    if (norm <= 1e-6F) {
        return;
    }

    for (float& value : feature) {
        value /= norm;
    }
}

float EmbeddingSimilarity(
    const std::vector<float>& embedding1,
    const std::vector<float>& embedding2
)
{
    if (embedding1.size() != embedding2.size() ||
        embedding1.empty()) {
        return 0.0F;
    }

    float cosine_similarity = 0.0F;

    for (std::size_t i = 0; i < embedding1.size(); ++i) {
        cosine_similarity += embedding1[i] * embedding2[i];
    }

    // 避免浮点误差导致结果超出[-1, 1]
    cosine_similarity = std::clamp(
        cosine_similarity,
        -1.0F,
        1.0F);

    return (cosine_similarity + 1.0F) / 2.0F;
}

}