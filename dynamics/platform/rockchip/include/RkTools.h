#ifndef _RK_TOOLS_H
#define _RK_TOOLS_H

#include <rknpu2/rknn_api.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace RkTools {
// 获取模型尺寸
bool GetInputSize(
    const rknn_tensor_attr& attr,
    int& width, 
    int& height,
    int& channels
);

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
