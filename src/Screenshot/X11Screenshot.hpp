#pragma once
#include "IScreenshot.hpp"

#include <X11/Xlib.h>

class X11Screenshot : public IScreenshot {
private:

    Display* m_display;

public:
    X11Screenshot();
    ~X11Screenshot() override;

    Display* getRawDisplay() override;
    bool captureToMemory(cv::Mat& image) override;
    std::string getActiveWindowId() override;
    std::string getActiveWindowProcessName() override;
};

