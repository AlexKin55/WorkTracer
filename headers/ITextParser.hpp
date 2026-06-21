#pragma once

#include <string>
#include <memory>

#include <opencv2/opencv.hpp>

class ITextParser {
public:
    virtual ~ITextParser() = default;
    virtual std::string parse(const cv::Mat&) = 0;
};