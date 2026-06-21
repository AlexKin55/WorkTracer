#pragma once
#include "IScreenshot.hpp"

#include <opencv2/opencv.hpp>

#include <string>

class MutterScreenshot : public IScreenshot {
public:
    MutterScreenshot();
    ~MutterScreenshot() override;

    bool captureToMemory(cv::Mat& image) override;
    std::string getActiveWindowId() override;
    std::string getActiveWindowProcessName() override;
};
