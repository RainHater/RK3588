#ifndef RKPLATFORM_SERVICE_FACE_PROCESSING_H
#define RKPLATFORM_SERVICE_FACE_PROCESSING_H

#include <opencv2/core/mat.hpp>

#include <vector>

namespace rkplatform::service::face {

// 图片预处理
cv::Mat PrepareFaceImage(const cv::Mat& bgr_image, int width, int height);

// 归一化
void NormalizeFeature(std::vector<float>& feature);

// 算 embedding 相似度
float EmbeddingSimilarity(
    const std::vector<float>& embedding1,
    const std::vector<float>& embedding2
);
}  // namespace rkplatform::service::face

#endif  // RKPLATFORM_SERVICE_FACE_PROCESSING_H
