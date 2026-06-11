#pragma once
#include "IScreenshot.hpp"
#include <memory>


class ScreenshotService {
private:
    std::unique_ptr<IScreenshot> engine;
    bool verbose;

public:
    ScreenshotService(std::unique_ptr<IScreenshot> screenshot_engine, bool enable_logging);
    bool capture_window(cv::Mat& out_frame);
    std::string get_current_window_id();
    std::string get_current_window_process_name();
};
