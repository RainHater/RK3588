#include "Logger.h"
#include "RknnInference.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

namespace {

constexpr const char* kDefaultModelPath =
    "/home/rainhater/UserData/Projects/Cpp/Base/RK3588/"
    "models/facenet-float.rknn";

constexpr const char* kDefaultImagePath =
    "/home/rainhater/UserData/Projects/Cpp/Base/RK3588/"
    "test/datasets/1_001.jpg";

bool GetInputSize(const rknn_tensor_attr& attr, int& width, int& height,
                  int& channels)
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

cv::Mat PrepareFaceImage(const cv::Mat& bgr_image, int width, int height)
{
    cv::Mat rgb_image;
    cv::cvtColor(bgr_image, rgb_image, cv::COLOR_BGR2RGB);

    cv::Mat resized_image;
    cv::resize(rgb_image, resized_image, cv::Size(width, height), 0.0, 0.0,
               cv::INTER_LINEAR);

    return resized_image.isContinuous() ? resized_image
                                        : resized_image.clone();
}

void NormalizeFeature(std::vector<float>& feature)
{
    float square_sum = 0.0F;
    for (const float value : feature) {
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

}  // namespace

int main(int argc, char* argv[])
{
    auto logger = LoggerWithTag::GetLogger("main");

    const std::string model_path = argc > 1 ? argv[1] : kDefaultModelPath;
    const std::string image_path = argc > 2 ? argv[2] : kDefaultImagePath;

    RknnInference inference;
    if (!inference.Initialize(model_path)) {
        logger->error("初始化FaceNet模型失败: {}", model_path);
        return 1;
    }

    const rknn_tensor_attr& input_attr = inference.GetInputAttr();
    int input_width = 0;
    int input_height = 0;
    int input_channels = 0;

    if (!GetInputSize(input_attr, input_width, input_height, input_channels)) {
        logger->error("不支持的模型输入格式, n_dims={}, fmt={}",
                      input_attr.n_dims, static_cast<int>(input_attr.fmt));
        return 1;
    }

    if (input_channels != 3) {
        logger->error("FaceNet输入通道数应为3, 当前为{}", input_channels);
        return 1;
    }

    cv::Mat bgr_image = cv::imread(image_path, cv::IMREAD_COLOR);
    if (bgr_image.empty()) {
        logger->error("读取人脸图片失败: {}", image_path);
        return 1;
    }

    cv::Mat input_image =
        PrepareFaceImage(bgr_image, input_width, input_height);

    std::vector<RknnOutput> outputs;
    const auto begin = std::chrono::steady_clock::now();

    const bool success = inference.Infer(
        outputs,
        input_image.data,
        input_image.total() * input_image.elemSize(),
        RKNN_TENSOR_UINT8,
        RKNN_TENSOR_NHWC,
        false);

    const auto end = std::chrono::steady_clock::now();

    if (!success) {
        logger->error("FaceNet推理失败");
        return 1;
    }

    if (outputs.empty()) {
        logger->error("FaceNet没有返回输出");
        return 1;
    }

    std::vector<float>& feature = outputs[0].data;
    if (feature.size() != 128) {
        logger->error("FaceNet输出维度错误, 期望128, 实际{}", feature.size());
        return 1;
    }

    NormalizeFeature(feature);

    const double elapsed_ms =
        std::chrono::duration<double, std::milli>(end - begin).count();

    logger->info("模型输入尺寸: {}x{}x{}", input_width, input_height,
                 input_channels);
    logger->info("人脸特征维度: {}", feature.size());
    logger->info("单次推理耗时: {} ms", elapsed_ms);

    for (std::size_t i = 0; i < feature.size(); ++i) {
        logger->info("feature[{}] = {}", i, feature[i]);
    }

    return 0;
}
