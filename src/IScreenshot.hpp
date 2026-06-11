#pragma once

#include <string>
#include <memory>
#include <opencv2/opencv.hpp>

class IScreenshot {
public:
    virtual ~IScreenshot() = default;
    virtual bool capture_to_memory(cv::Mat& out_image) = 0;
    virtual std::string get_active_window_process_name() = 0; 
    virtual std::string get_active_window_id_str() = 0;
};