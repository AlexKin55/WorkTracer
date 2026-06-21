#include "ScreenshotService.hpp"

ScreenshotService::ScreenshotService(std::unique_ptr<IScreenshot> screenshotEngine, const AppConfig& cfg)
    : m_engine(std::move(screenshotEngine)), m_cfg(cfg), m_verbose(m_cfg.enableLogging) {
}

bool ScreenshotService::captureWindow(cv::Mat& frame) {
    if (!m_engine) {
        return false;
    }

    std::string processName = m_engine->getActiveWindowProcessName();
    std::string processLower = processName;
    std::transform(processLower.begin(), processLower.end(), processLower.begin(), ::tolower);

    const auto& ignoredList = m_cfg.ignoredProcesses;
    if (ignoredList.find(processLower) != ignoredList.end()) {
        std::cout << "[ScreenshotService] Capture skipped. Process '" 
                  << processName << "' is in the ignore list." << std::endl;
        return false;
    }

    std::string winId = m_engine->getActiveWindowId();
    if (winId == "0" || winId.empty()) {
        return false;
    }

    bool success = m_engine->captureToMemory(frame);
    
    if (success && m_verbose) {
        std::cout << "[ScreenshotService] The window was captured successfully. ID: " << winId 
                    << " (" << frame.cols << "x" << frame.rows << " px в RAM)" << std::endl;
    }
    return success;
}

std::string ScreenshotService::getCurrentWindowId() {
    return m_engine ? m_engine->getActiveWindowId() : "0";
}

std::string ScreenshotService::getCurrentWindowProcessName() {
    return m_engine ? m_engine->getActiveWindowProcessName() : "unknown";
}

