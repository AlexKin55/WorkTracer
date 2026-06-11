#pragma once

#include "IScreenshot.hpp"
#include <X11/Xlib.h>

class X11Screenshot : public IScreenshot {
private:

    Display* display;
    Window get_active_window_id();

public:
    X11Screenshot();
    ~X11Screenshot() override;

    Display* get_raw_display();
    bool capture_to_memory(cv::Mat& out_image) override;
    std::string get_active_window_id_str() override;
    std::string get_active_window_process_name() override;
};

