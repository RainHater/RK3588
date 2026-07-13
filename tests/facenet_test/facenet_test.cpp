
#include "Logger.h"
#include "Tools.h"
#include "RknnInference.h"

#include <opencv2/imgcodecs.hpp>

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[])
{
    if (argc != 4) {
        std::cerr
            << "用法: "
            << argv[0]
            << " <model> <image1> <image2>\n";

        return 1;
    }

    const std::string model_path = argv[1];
    const std::string image1_path = argv[2];
    const std::string image2_path = argv[3];
    
    auto logger = LoggerWithTag::GetLogger("main");
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

    logger->info("输入通道: {}, 输入宽度: {}, 输入高度: {}", input_channels, input_width, input_height);

    if (input_channels != 3) {
        logger->error("FaceNet输入通道数应为3, 当前为{}", input_channels);
        return 1;
    }

    auto GetInfEmbedding = [&logger, &inference, &input_width, &input_height](std::string image_path, std::vector<float>& embedding){
        cv::Mat bgr_image = cv::imread(image_path, cv::IMREAD_COLOR);

        if (bgr_image.empty()) {
            logger->error("读取人脸图片失败: {}", image_path);
            return false;
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
            return false;
        }

        if (outputs.empty()) {
            logger->error("FaceNet没有返回输出");
            return false;
        }
        
        embedding = outputs[0].data;
        logger->info("推理成功, 推理时间: {} ms", elapsed_ms);

        return true;
    };

    std::vector<float> embedding1;
    std::vector<float> embedding2;

    if (!GetInfEmbedding(image1_path, embedding1)){
        logger->error("推理失败!");
        return 1;
    }

    if (!GetInfEmbedding(image2_path, embedding2)){
        logger->error("推理失败!");
        return 1;
    }

    float similarity = RkTools::EmbeddingSimilarity(embedding1, embedding2);
    logger->info("相同概率: {}", similarity);

    return 0;
}