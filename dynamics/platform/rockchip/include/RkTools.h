#ifndef _RK_TOOLS_H
#define _RK_TOOLS_H

#include <rknpu2/rknn_api.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace RkTools {
// 图片预处理
cv::Mat PrepareFaceImage(const cv::Mat& bgr_image, int width, int height);

// 归一化
void NormalizeFeature(std::vector<float>& feature);

// 算 embedding 相似度
float EmbeddingSimilarity(
    const std::vector<float>& embedding1,
    const std::vector<float>& embedding2
);
}

#endif
