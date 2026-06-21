#pragma once
#include "IScreenshot.hpp"

#include <opencv2/opencv.hpp>
#include <string>

class KWinScreenshot : public IScreenshot {
public:
    KWinScreenshot();
    ~KWinScreenshot() override;

    bool captureToMemory(cv::Mat& image) override;
    std::string getActiveWindowId() override;
    std::string getActiveWindowProcessName() override;
};

