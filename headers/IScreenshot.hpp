#pragma once

#include <string>
#include <memory>

#include <opencv2/opencv.hpp>

struct _XDisplay;
typedef struct _XDisplay Display;

class IScreenshot {
public:
    virtual ~IScreenshot() = default;
    virtual bool captureToMemory(cv::Mat&) = 0;
    virtual std::string getActiveWindowProcessName() = 0; 
    virtual std::string getActiveWindowId() = 0;
    virtual Display* getRawDisplay() {
        return nullptr;
    };
};