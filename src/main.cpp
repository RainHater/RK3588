#include "Logger.h"
#include "RknnInference.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <chrono>
#include <cmath>
#include <cstddef>
#include <string>
#include <vector>

constexpr const char* kDefaultModelPath =
    "/home/rainhater/UserData/Projects/Cpp/Base/RK3588/"
    "models/facenet-float.rknn";

constexpr const char* k1DefaultImagePath =
    "/home/rainhater/UserData/Projects/Cpp/Base/RK3588/"
    "test/datasets/1_001.jpg";

constexpr const char* k2DefaultImagePath =
    "/home/rainhater/UserData/Projects/Cpp/Base/RK3588/"
    "test/datasets/1_002.jpg";

int main(int argc, char* argv[]){
    auto logger = LoggerWithTag::GetLogger("main");

    const std::string model_path = kDefaultModelPath;
    const std::string image_1_path = k1DefaultImagePath;
    const std::string image_2_path = k2DefaultImagePath;

    RknnInference inference;
    if (!inference.Initialize(model_path)) {
        logger->error("初始化FaceNet模型失败: {}", model_path);
        return 1;
    }

    const rknn_tensor_attr& input_attr = inference.GetInputAttr();
    int input_width = 0;
    int input_height = 0;
    int input_channels = 0;

    if (!RkTools::GetInputSize(input_attr, input_width, input_height, input_channels)) {
        logger->error("不支持的模型输入格式, n_dims={}, fmt={}",
                      input_attr.n_dims, static_cast<int>(input_attr.fmt));
        return 1;
    }

    if (input_channels != 3) {
        logger->error("FaceNet输入通道数应为3, 当前为{}", input_channels);
        return 1;
    }

    std::vector<std::string> image_path_v = {image_1_path, image_2_path};
    std::vector<std::vector<float>> feature;
    
    for (int i = 0; i < 2; i ++){
        cv::Mat bgr_image = cv::imread(image_path_v[i], cv::IMREAD_COLOR);
        if (bgr_image.empty()) {
            logger->error("读取人脸图片失败: {}", image_path_v[i]);
            return 1;
        }
        cv::Mat input_image = RkTools::PrepareFaceImage(bgr_image, input_width, input_height);
        std::vector<RknnOutput> outputs;
        const auto begin = std::chrono::steady_clock::now();
        const bool success = inference.Infer(
            outputs,
            input_image.data,
            input_image.total() * input_image.elemSize(),
            RKNN_TENSOR_UINT8,
            RKNN_TENSOR_NHWC,
            false
        );

        const auto end = std::chrono::steady_clock::now();
        const double elapsed_ms = std::chrono::duration<double, std::milli>(end - begin).count();

        if (!success) {
            logger->error("FaceNet推理失败");
            return 1;
        }

        if (outputs.empty()) {
            logger->error("FaceNet没有返回输出");
            return 1;
        }

        feature.push_back(outputs[0].data);

        RkTools::NormalizeFeature(feature[i]);

        logger->info("模型输入尺寸: {}x{}x{}", input_width, input_height, input_channels);
        logger->info("人脸特征维度: {}", feature.size());
        logger->info("单次推理耗时: {} ms", elapsed_ms);
    }

    float similarity = RkTools::EmbeddingSimilarity(feature[0], feature[1]);

    logger->info("相同概率: {}", similarity);

    return 0;
}
