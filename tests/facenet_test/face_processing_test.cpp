#include "FaceProcessing.h"

#include <opencv2/core.hpp>

#include <cmath>
#include <iostream>
#include <vector>

namespace {

bool IsNear(float actual, float expected) {
    return std::fabs(actual - expected) < 1e-5F;
}

}  // namespace

int main() {
    std::vector<float> feature{3.0F, 4.0F};
    rkplatform::service::face::NormalizeFeature(feature);

    if (!IsNear(feature[0], 0.6F) || !IsNear(feature[1], 0.8F)) {
        std::cerr << "特征归一化结果错误\n";
        return 1;
    }

    const float similarity =
        rkplatform::service::face::EmbeddingSimilarity(feature, feature);
    if (!IsNear(similarity, 1.0F)) {
        std::cerr << "相似度计算结果错误\n";
        return 1;
    }

    cv::Mat bgr(2, 3, CV_8UC3, cv::Scalar(1, 2, 3));
    const cv::Mat rgb = rkplatform::service::face::PrepareFaceImage(bgr, 4, 5);
    if (rgb.cols != 4 || rgb.rows != 5 || rgb.type() != CV_8UC3) {
        std::cerr << "人脸图像预处理尺寸或格式错误\n";
        return 1;
    }

    return 0;
}
