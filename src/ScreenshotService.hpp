#pragma once
#include "IScreenshot.hpp"
#include "ConfigManager.hpp"

#include <memory>


class ScreenshotService {
private:
    std::unique_ptr<IScreenshot> m_engine;
    const AppConfig& m_cfg;
    bool m_verbose;

public:
    ScreenshotService(std::unique_ptr<IScreenshot> screenshotEngine, const AppConfig& cfg);
    bool captureWindow(cv::Mat& frame);
    std::string getCurrentWindowId();
    std::string getCurrentWindowProcessName();
};
